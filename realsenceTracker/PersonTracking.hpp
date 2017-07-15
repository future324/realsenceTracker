#pragma once
#include<pxcpersontrackingconfiguration.h>
#include<pxcpersontrackingdata.h>
#include<pxcpersontrackingmodule.h>
#include<pxcprojection.h>
#include<pxcpowerstate.h>
#include<utilities\pxcsmoother.h>
#include<opencv2/opencv.hpp>
#include<Windows.h>
using namespace std;
class PersonTracking
{
public:
	bool Init() {
		sr300_manager = PXCSenseManager::CreateInstance();
		sr300_manager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 960, 540, 60);
		sr300_manager->EnablePersonTracking();
		trackingModule = sr300_manager->QueryPersonTracking();
		PXCPersonTrackingConfiguration *pertrackingConfig = trackingModule->QueryConfiguration();
		pertrackingConfig->SetTrackedAngles(PXCPersonTrackingConfiguration::TRACKING_ANGLES_ALL);
		//persondata = trackingModule->QueryOutput();
		sr300_manager->Init(); 
		Update();
		return true;
	}
	int Update() {
		// 清零
		sr300_manager->ReleaseFrame();
		// 更新
		if (sr300_manager->AcquireFrame(true) < PXC_STATUS_NO_ERROR) return -1;
		sample = sr300_manager->QuerySample();
		color = sample->color;
		persondata = trackingModule->QueryOutput();
		npersons = persondata->QueryNumberOfPeople();
		for (size_t i = 0; i < npersons; i++) {
			person[i] = persondata->QueryPersonData(PXCPersonTrackingData::AccessOrderType::ACCESS_ORDER_BY_ID, i)->QueryTracking();
		}
		return npersons;
	}
	DWORD KillDCM()
	{
		cout << "正在重启DCM[";
		SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		sei.lpVerb = TEXT("runas");
		sei.lpFile = TEXT("cmd.exe");
		sei.nShow = SW_HIDE;
		// 停止
		sei.lpParameters = TEXT("cmd /c NET STOP RealSenseDCMSR300 & NET START RealSenseDCMSR300");

		if (!ShellExecuteEx(&sei)) {
			if (GetLastError() == ERROR_CANCELLED)
				cout << "访问权限被用户拒绝";
		}
		DWORD dwExitCode;
		GetExitCodeProcess(sei.hProcess, &dwExitCode);
		while (dwExitCode == STILL_ACTIVE) {
			Sleep(800);
			GetExitCodeProcess(sei.hProcess, &dwExitCode);
			cout << "=";
		}
		cout << "]";
		CloseHandle(sei.hProcess);
		dwExitCode ? cout << "访问错误(" << dwExitCode << ")" << endl : cout << "完成" << endl;
		return dwExitCode;
	}
	
	cv::Mat QueryColorImage() {
		auto pxcimg = sample->color;
		auto info = pxcimg->QueryInfo();
		PXCImage::ImageData img_dat;
		pxcimg->AcquireAccess(PXCImage::Access::ACCESS_READ, PXCImage::PIXEL_FORMAT_RGB24, &img_dat);
		cv::Mat img = cv::Mat(info.height, info.width, CV_8UC3, (void*)img_dat.planes[0], img_dat.pitches[0] / sizeof(uchar));
		//	cv::normalize(img,img,0,255)
		pxcimg->ReleaseAccess(&img_dat);
		//flip(img*1.5, img, 1);
		pyrDown(img, img);
		return img;
	}

	cv::Point3f QueryMassCenterWorld(PXCPersonTrackingData::PersonTracking *person) {
		if (person == NULL)return cv::Point3f(0,0,0);
		auto worldPos = person->QueryCenterMass().world.point;
		return cv::Point3f(worldPos.x, worldPos.y, worldPos.z);
	}
	PXCPersonTrackingData::PersonTracking *Queryperson()
	{
		return persondata->QueryPersonData(PXCPersonTrackingData::AccessOrderType::ACCESS_ORDER_BY_ID,0)->QueryTracking();
		// person[PXCPersonTrackingData::AccessOrderType::ACCESS_ORDER_BY_ID];
	}
	cv::Rect QueryHandBoundingBox(PXCPersonTrackingData::PersonTracking *person)
	{
		auto rect = person->Query2DBoundingBox().rect;
		if (person!=NULL)
		{
			return cv::Rect(rect.x, rect.y, rect.w, rect.h);
		}
		
	}
	cv::Mat draw(PXCPersonTrackingData::PersonTracking *person)
	{

		if (person!=NULL)
		{
			auto box = QueryHandBoundingBox(person);
			rectangle(QueryColorImage(), box, cv::Scalar(0, 255, 255),1);
		}
		
		return QueryColorImage();
	}
private:
	PXCSenseManager* sr300_manager = NULL;
	PXCImage *color, *depth, *segmask = NULL;
	PXCCapture::Sample *sample=NULL;
	PXCPersonTrackingModule *trackingModule = NULL;
	PXCPersonTrackingData *persondata=NULL;
	PXCPersonTrackingData::PersonTracking *person[5] = {};
	PXCSmoother *smooth = NULL;
	pxcI32 npersons = 0;
};
