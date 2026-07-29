#include <iostream>

extern "C"
{
#include "libavcodec/avcodec.h"
}

const char* inFileName = "D:\\0_edoyun\\企业项目实战\\CloudTeachingAssistant\\VideoEncode\\output.yuv";
const char* outFileName = "output.h264";

int nEncode(AVCodecContext* codecContext, AVPacket* packet, AVFrame* frame, FILE* outfile)
{
	int nRet = 0;
	nRet = avcodec_send_frame(codecContext, frame);
	if (nRet < 0)
	{
		std::cout << "avcodec_send_frame fail" << std::endl;
		return nRet;
	}

	while (nRet == 0)
	{
		nRet = avcodec_receive_packet(codecContext, packet);
		if (nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF)
		{
			return 0;
		}
		else if (nRet < 0)
		{
			std::cout << "avcodec_receive_packet fail" << std::endl;
			return nRet;
		}
		else if (nRet == 0)
		{
			fwrite(packet->data, 1, packet->size, outfile);
		}
	}

	return 0;
}

int main()
{
	int nRet = 0;
	//准备编码器
	const AVCodec* codec = nullptr;
	AVCodecContext* codecContext = nullptr;
	AVPacket* packet = nullptr;
	AVFrame* frame = nullptr;

	FILE* inFile = nullptr;
	FILE* outFile = nullptr;

	//查找264编码器
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec)
	{
		std::cout << "avcodec_find_encoder fail" << std::endl;
		return -1;
	}

	//分配编码器上下文
	codecContext = avcodec_alloc_context3(codec);
	if (!codecContext)
	{
		std::cout << "avcodec_alloc_context3 fail" << std::endl;
		return -2;
	}

	//设置上下文参数
	codecContext->width = 1280;
	codecContext->height = 720;
	codecContext->time_base = AVRational{ 1, 25 };
	codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	codecContext->framerate = AVRational{ 25, 1 };

	//打开编码器
	nRet = avcodec_open2(codecContext, codec, NULL);
	if (nRet != 0)
	{
		std::cout << "avcodec_open2 fail" << std::endl;
		return -3;
	}

	packet = av_packet_alloc();
	if (!packet)
	{
		std::cout << "av_packet_alloc fail" << std::endl;
		return -4;
	}

	frame = av_frame_alloc();
	if (!frame)
	{
		std::cout << "av_frame_alloc fail" << std::endl;
		return -5;
	}

	//设置参数
	frame->width = 1280;
	frame->height = 720;
	frame->format = AV_PIX_FMT_YUV420P;

	//申请frame的内存来存放帧数据
	nRet = av_frame_get_buffer(frame, 0);
	if (nRet != 0)
	{
		std::cout << "av_frame_get_buffer fail" << std::endl;
		return -6;
	}

	//打开输入文件读数据
	inFile = fopen(inFileName, "rb");
	if (!inFile)
	{
		std::cout << "infile open fail" << std::endl;
		return -7;
	}

	//打开输出文件写数据
	outFile = fopen(outFileName, "wb");
	if (!inFile)
	{
		std::cout << "outFile open fail" << std::endl;
		return -8;
	}

	//循环读数据编码
	while (!feof(inFile))
	{
		//frame能不能写，需要将yuv数据填充至frame，检查是否可写
		nRet = av_frame_is_writable(frame);
		if (nRet < 0)
		{
			//设置可写
			nRet = av_frame_make_writable(frame);
		}

		//从yuv文件中读取
		//y分量
		fread(frame->data[0], 1, frame->width * frame->height, inFile);
		//u分量
		fread(frame->data[1], 1, frame->width * frame->height / 4, inFile);
		//v分量
		fread(frame->data[2], 1, frame->width * frame->height / 4, inFile);

		//编码
		nEncode(codecContext, packet, frame, outFile);
	}

	//读完就跳出循环，刷新编码器
	nEncode(codecContext, packet, nullptr, outFile);
	std::cout << "编码完成" << std::endl;

	//释放资源
	av_packet_free(&packet);
	av_frame_free(&frame);
	avcodec_free_context(&codecContext);
	fclose(inFile);
	fclose(outFile);

	return 0;
}