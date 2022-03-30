#pragma once

#include "pch.h"

class Math
{
public:

	wchar_t* ctowc(IN const char* str)
	{
		if (str == nullptr)
		{
			return nullptr;
		}
		DWORD num = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
		wchar_t* pRes = new wchar_t[num];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, pRes, num);
		return pRes;
	}

	template< typename... Args >
	std::string string_sprintf(const char* format, Args... args)
	{
		int length = std::snprintf(nullptr, 0, format, args...);
		assert(length >= 0);

		char* buf = new char[length + 1];
		std::snprintf(buf, length + 1, format, args...);

		std::string str(buf);
		delete[] buf;
		return std::move(str);
	}

	float distance_pp(cv::Point2f pa, cv::Point2f pb)
	{
		return static_cast<float>(std::sqrt(std::pow(pa.x - pb.x, 2) + std::pow(pa.y - pb.y, 2)));
	}

	bool calc_intersection_point(cv::Point pa, cv::Point pb,
		cv::Point2f pc, cv::Point2f pd,
		cv::Point2f& p)
	{
		/// 三角形abc 面积的2倍
		float area_abc = (pa.x - pc.x) * (pb.y - pc.y) - (pa.y - pc.y) * (pb.x - pc.x);
		/// 三角形abd 面积的2倍
		float area_abd = (pa.x - pd.x) * (pb.y - pd.y) - (pa.y - pd.y) * (pb.x - pd.x);
		/// 面积符号相同则两点在线段同侧,不相交 (对点在线段上的情况,本例当作不相交处理);
		if (area_abc * area_abd > Zero)
			return false;

		/// 三角形cda 面积的2倍  
		float area_cda = (pc.x - pa.x) * (pd.y - pa.y) - (pc.y - pa.y) * (pd.x - pa.x);
		/// 三角形cdb 面积可用其他三个三角形的面积求得，这里考虑了各个三角形面积的正负号
		float area_cdb = area_cda + area_abc - area_abd;
		/// 面积符号相同则两点在线段同侧,不相交 (对点在线段上的情况,本例当作不相交处理);
		if (area_cda * area_cdb > Zero)
			return false;

		float t = area_cda / (area_abd - area_abc);
		p.x = t * (pb.x - pa.x) + pa.x;
		p.y = t * (pb.y - pa.y) + pa.y;

		return true;
	}

	int calc_rotaterect_step(cv::RotatedRect& rotate_rect)
	{
		cv::Point2f point[4];
		rotate_rect.points(point);

		/// opencv 3.4.1和4.5.2版本 关于RotatedRect.points定义不同
		/// opencv 3.4.1 point[0]是y坐标最大值，如果矩形平行，则x值最小点是[0]
		/// opencv 4.5.2 point[3]是y坐标最大值，如果矩形平行，则x值最小点是[3]
		/// rotate_rect.size.width = distance_pp(point[0], point[3])
		/// rotate_rect.size.height = distance_pp(point[0], point[1])
		return point[3].x > point[1].x;
	}

	cv::Mat HObject2Mat(IN HObject& ho_Image)
	{
		cv::Mat img;
		HTuple  hv_Channels;

		ConvertImageType(ho_Image, &ho_Image, "byte");
		CountChannels(ho_Image, &hv_Channels);
		if (hv_Channels.I() == 1)
		{
			HTuple hv_Ptr, hv_Type, hv_Width, hv_Height;
			GetImagePointer1(ho_Image, &hv_Ptr, &hv_Type, &hv_Width, &hv_Height);

			img = cv::Mat(cv::Size(hv_Width, hv_Height), CV_8UC1);
			unsigned char *pdata = (unsigned char *)hv_Ptr.L();
			memcpy(img.data, pdata, hv_Width.I() * hv_Height.I());
		}
		else if (hv_Channels.I() == 3)
		{
			HTuple  hv_PtrR, hv_PtrG, hv_PtrB, hv_Type, hv_Width, hv_Height;
			GetImagePointer3(ho_Image, &hv_PtrR, &hv_PtrG, &hv_PtrB, &hv_Type, &hv_Width, &hv_Height);

			img = cv::Mat(cv::Size(hv_Width, hv_Height), CV_8UC3);

			std::vector<cv::Mat> vecM(3);
			vecM[0] = cv::Mat(cv::Size(hv_Width, hv_Height), CV_8UC1);
			vecM[1] = cv::Mat(cv::Size(hv_Width, hv_Height), CV_8UC1);
			vecM[2] = cv::Mat(cv::Size(hv_Width, hv_Height), CV_8UC1);
			unsigned char *R = (unsigned char *)hv_PtrR.L();
			unsigned char *G = (unsigned char *)hv_PtrG.L();
			unsigned char *B = (unsigned char *)hv_PtrB.L();
			memcpy(vecM[2].data, R, hv_Width.I() * hv_Height.I());
			memcpy(vecM[1].data, G, hv_Width.I() * hv_Height.I());
			memcpy(vecM[0].data, B, hv_Width.I() * hv_Height.I());
			cv::merge(vecM, img);
		}
		return img;
	}

	HObject Mat2HObject(IN cv::Mat& img)
	{
		HObject ho_Image;
		if (img.channels() == 4)
		{
			cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);
		}
		if (img.channels() == 1)
		{
			GenImage1(&ho_Image, "byte", img.cols, img.rows, (Hlong)img.data);
		}
		else if (img.channels() == 3)
		{
			std::vector<cv::Mat> vecM;
			cv::split(img, vecM);
			cv::Mat imgB = vecM[0];
			cv::Mat imgG = vecM[1];
			cv::Mat imgR = vecM[2];
			GenImage3(&ho_Image, "byte", img.cols, img.rows, (Hlong)imgR.data, (Hlong)imgG.data, (Hlong)imgB.data);
		}

		return ho_Image;
	}	
	
public:
	static Math& GetInstance()
	{
		static Math m_instance;
		static std::once_flag oc;
		std::call_once(oc, []
		{
			SetSystem("reentrant", "true");
		});
		return m_instance;
	}
};
