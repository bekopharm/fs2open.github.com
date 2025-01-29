
#ifndef HEADTRACKING_FACETRACKNOIR_H
#define HEADTRACKING_FACETRACKNOIR_H

#include "headtracking/headtracking.h"
#include "headtracking/headtracking_internal.h"

#include "external_dll/externalcode.h"


namespace headtracking
{
	namespace facetracknoir
	{
		#define MAXLINE 48 

		struct FaceTrackNoIRData
		{
			double x;
			double y;
			double z;
			double yaw;
			double pitch;
			double roll;
			long frameNumber;
		};

		class FaceTrackNoIRLibrary : public SCP_ExternalCode
		{
		private:
			int sockfd;
			bool mEnabled;

			bool StartSocket(void);


		public:
			FaceTrackNoIRLibrary();
			FaceTrackNoIRData data;

			virtual ~FaceTrackNoIRLibrary() {}

			//bool GetData(FaceTrackNoIRData * data);
			bool GetData(void);

			char* Provider(void);

			bool Enabled() const { return mEnabled; }
		};

		class FaceTrackNoIRProvider : public internal::HeadTrackingProvider
		{
		private: 
			FaceTrackNoIRLibrary library;

		public:
			FaceTrackNoIRProvider();

			virtual ~FaceTrackNoIRProvider();

			bool query(HeadTrackingStatus* statusOut) override;

			static SCP_string getName();
		};
	}
}

#endif // HEADTRACKING_FACETRACKNOIR_H
