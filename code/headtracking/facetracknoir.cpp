#include "headtracking/facetracknoir.h"

#include <string.h>
#ifndef WIN32
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
#else
# include <winsock2.h>
# include <ws2tcpip.h> /*sockaddr, addrinfo etc.*/
# include <stdint.h>
#endif /*WIN32*/

// default port https://facetracknoir.sourceforge.net/Protocols/FaceTrackNoIR.htm
#define PORT 5550 
#define MAXLINE 48 

namespace headtracking
{

	namespace facetracknoir
	{
		const char* const mFTNIRProvider = "FaceTrackNoIR UDP";

		int GetDataThread(void *obj)
		{
			//((FaceTrackNoIRLibrary *)obj)->WorkerThread();
			//((FaceTrackNoIRLibrary *)obj)->m_Aborted = true;
			// ((FaceTrackNoIRLibrary *)obj)->data;
			printf("GetDataThread v2 started\n");


			while(((FaceTrackNoIRLibrary *)obj)->GetData()) {
				// printf("GetDataThread executing\n");
				// do we need a sleep? don't think so.
				// sleep(0.1);
			}

			printf("GetDataThread v2 stopped\n");

			return 1;
		}


		FaceTrackNoIRLibrary::FaceTrackNoIRLibrary(): sockfd(0), mEnabled(false)
		{
			// set initial values
			// FaceTrackNoIRData data;
			data.x = 0;
			data.y = 0;
			data.z = 0;
			data.yaw = 0;
			data.pitch = 0;
			data.roll = 0;
			data.frameNumber = 0;

			mEnabled = true;
		}

		bool FaceTrackNoIRLibrary::StartSocket(void)
		{
			sockaddr_in cto;

			if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) 
			{ 
				throw internal::HeadTrackingException("Socket creation failed!");
				mEnabled = false;
				return false;
			} 

			memset(&cto, 0, sizeof(cto)); 

			cto.sin_family = AF_INET;
			cto.sin_addr.s_addr = INADDR_ANY;
			cto.sin_port = htons(PORT);

			// Bind the socket with the server address 
			if ( bind(sockfd, (const struct sockaddr *)&cto, sizeof(cto)) < 0 )
			{ 
				throw internal::HeadTrackingException("Socket bind failed!");
				mEnabled = false;
				return false;
			} 

			mprintf(("Socket created on port '%i'.\n", PORT));
			mEnabled = true;
			return true;
		}


		bool FaceTrackNoIRLibrary::GetData(void /**FaceTrackNoIRData * data**/)
		{
			// printf("GetData executing\n");
			// TODO: decide when to reset if nothing of value can be read from the socket
			if(!mEnabled || sockfd <= 0) {
				return this->StartSocket();
			}

			char buffer[MAXLINE];
			sockaddr_in cfrom;
			socklen_t len;
			int n;
			len = sizeof(cfrom);
			memset(&cfrom, 0, len);

			// MSG_DONTWAIT
			n = recvfrom(sockfd, (char *)buffer, sizeof(buffer), MSG_WAITALL, ( struct sockaddr *) &cfrom, &len);

			if(n != sizeof(buffer))
			{
				// false kills reading forever
				return true;
			}

			// buffer[n] = '\0'; 

			static_assert( sizeof(double) == 8 );

			double x = *reinterpret_cast<double*>(buffer);
			if(std::isnan(x)) return true;
			double y = *reinterpret_cast<double*>(buffer + 8);
			if(std::isnan(y)) return true;
			double z = *reinterpret_cast<double*>(buffer + 16);
			if(std::isnan(z)) return true;
			double yaw = *reinterpret_cast<double*>(buffer + 24);
			if(std::isnan(yaw)) return true;
			double pitch = *reinterpret_cast<double*>(buffer + 32);
			if(std::isnan(pitch)) return true;
			double roll = *reinterpret_cast<double*>(buffer + 40);
			if(std::isnan(roll)) return true;

			data.x = x;
			data.y = y;
			data.z = z;
			data.yaw = yaw;
			data.pitch = pitch;
			data.roll = roll;
			data.frameNumber++;


			// printf("UDP socket:   x: %f     y: %f    z: %f\n", data.x, data.y,  data.z);
			// printf("UDP socket: yaw: %f pitch: %f roll: %f\n", data.yaw, data.pitch, data.roll);


			// printf("Yaw %f, pitch: %f, roll: %f \n", data.yaw, data.pitch, data.roll);
			return true;
			
			//if (mFTNIRGetData)
			//	return mFTNIRGetData(data);

		}

		char* FaceTrackNoIRLibrary::Provider(void)
		{
			if (mFTNIRProvider) {
				char * provider = strdup(mFTNIRProvider);
				return provider;
			}

			return nullptr;
		}

		FaceTrackNoIRProvider::FaceTrackNoIRProvider(): library(FaceTrackNoIRLibrary())
		{
			if (!library.Enabled())
			{
				throw internal::HeadTrackingException("Library could not be loaded!");
			}




			// call once manually to make sure this works
			//if (!library.GetData(&data))
			/*
			if (!library.GetData())
			{
				throw internal::HeadTrackingException("Failed to get test data set!");
			}
			*/


			SDL_Thread *thread = SDL_CreateThread(GetDataThread, "GetDataThread", this);

			if(thread == nullptr)
			{
				throw internal::HeadTrackingException("Library could not spawn thread!");
			}
			else
			{
				SDL_DetachThread(thread);
			}

			mprintf(("Found FaceTrackNoIR provider '%s'.\n", library.Provider()));
		}

		FaceTrackNoIRProvider::~FaceTrackNoIRProvider()
		{
		}

		bool FaceTrackNoIRProvider::query(HeadTrackingStatus* statusOut)
		{
			// map the FaceTrackNoIR / OpenTrack data to fs2open data
			// TODO: pitch seems to be yaw for OpenTrack and needs invert - needs confirmation!
			statusOut->pitch = (float) (library.data.yaw / 1000.0f);
			// TODO: yaw seems to be z for OpenTrack and needs invert - needs confirmation!
			statusOut->yaw = (float) (library.data.z / 1000.0f);
			statusOut->roll =(float) library.data.roll / 1000.0f ;

			// Coordinates are in millimeters
			statusOut->x = (float) library.data.x / 1000.0f;
			statusOut->y = (float) library.data.y / 1000.0f;
			// TODO: z seems to be pitch for OpenTrack - needs confirmation!
			statusOut->z = (float) library.data.pitch / 1000.0f;


//			printf("Client:   x: %f     y: %f    z: %f\n", statusOut->x, statusOut->y,  statusOut->z);
//			printf("Client: yaw: %f pitch: %f roll: %f\n", statusOut->yaw, statusOut->pitch, statusOut->roll);

			return true;
		}

		SCP_string FaceTrackNoIRProvider::getName()
		{
			return "FaceTrackNoIR";
		}
	}
}