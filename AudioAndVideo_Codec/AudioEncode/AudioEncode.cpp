#include <iostream>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
}

const char* inFileName = "D:\\0_edoyun\\企业项目实战\\CloudTeachingAssistant\\AudioAndVideo_Codec\\output.pcm";
const char* outFileName = "output.aac";

int main()
{
	//准备编码器
	const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!codec)
	{
		std::cout << "avcodec_find_encoder fail" << std::endl;
		return -1;
	}

	//准备编码上下文
	AVCodecContext* codecContext = avcodec_alloc_context3(codec);
	if (!codecContext)
	{
		std::cout << "avcodec_alloc_context3 fail" << std::endl;
		return -2;
	}

	//设置上下文
	codecContext->sample_rate = 44100;
	codecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
	av_channel_layout_default(&codecContext->ch_layout, 2);
	codecContext->bit_rate = 64000;

	//打开解码器
	int nRet = 0;
	nRet = avcodec_open2(codecContext, codec, NULL);
	if (nRet != 0)
	{
		std::cout << "avcodec_open2 fail" << std::endl;
		return -3;
	}

	//创建输出上下文
	AVFormatContext* formatContext = nullptr;
	avformat_alloc_output_context2(&formatContext, NULL, NULL, outFileName);
	if (!formatContext)
	{
		std::cout << "avformat_alloc_output_context2 fail" << std::endl;
		return -4;
	}

	//创建流
	AVStream* st = avformat_new_stream(formatContext, NULL);
	if (!st)
	{
		std::cout << "avformat_new_stream fail" << std::endl;
		return -5;
	}

	//流获取编码器参数
	avcodec_parameters_from_context(st->codecpar, codecContext);

	//打开输出文件
	nRet = avio_open(&formatContext->pb, outFileName, AVIO_FLAG_WRITE);
	if (nRet < 0)
	{
		std::cout << "avio_open fail" << std::endl;
		return -6;
	}

	avformat_write_header(formatContext, NULL);

	//初始化重采样
	SwrContext* swrContext = nullptr;
	AVChannelLayout avInputChannelLayout = AV_CHANNEL_LAYOUT_STEREO;
	nRet = swr_alloc_set_opts2(&swrContext, &codecContext->ch_layout, codecContext->sample_fmt,
		codecContext->sample_rate,&avInputChannelLayout, AV_SAMPLE_FMT_S16, 44100, 0, 0);
	if (nRet != 0)
	{
		std::cout << "swr_alloc_set_opts2 fail" << std::endl;
		return -7;
	}

	nRet = swr_init(swrContext);
	if (nRet < 0)
	{
		std::cout << "swr_init fail" << std::endl;
		return -8;
	}

	//设置Frame
	AVFrame* frame = nullptr;
	frame = av_frame_alloc();
	frame->format = AV_SAMPLE_FMT_FLTP;
	av_channel_layout_default(&frame->ch_layout, 2);
	frame->nb_samples = 1024;

	//分配内存
	nRet = av_frame_get_buffer(frame, 0);
	if (nRet < 0)
	{
		std::cout << "av_frame_get_buffer fail" << std::endl;
		return -9;
	}

	//开始读pcm->frame
	int nReadSize = frame->nb_samples * 2 * 2;//每次读的大小
	char* pcms = new char[nReadSize];
	FILE* fp = fopen(inFileName, "rb");
	if (!fp)
	{
		std::cout << "fopen fail" << std::endl;
		return -10;
	}

	//开始编码
	while (true)
	{
		AVPacket* packet = av_packet_alloc();
		int nReadLen = fread(pcms, nReadSize, 1, fp);
		if (nReadLen <= 0)
		{
			//数据读完了， flush_encoder
			avcodec_send_frame(codecContext, nullptr);
			while (avcodec_receive_packet(codecContext, packet) != AVERROR_EOF);
			break;
		}
		else
		{
			//开始重采样
			const uint8_t* data[1];
			data[0] = (uint8_t*)pcms;
			nRet = swr_convert(swrContext, frame->data, frame->nb_samples, data, frame->nb_samples);
			if (nRet < 0)
			{
				std::cout << "swr_convert fail" << std::endl;
				break;
			}

			//采样成功，编码数据
			nRet = avcodec_send_frame(codecContext, frame);
			if (nRet < 0)
			{
				std::cout << "avcodec_send_frame fail" << std::endl;
				continue;
			}

			//接收
			nRet = avcodec_receive_packet(codecContext, packet);
			if (nRet == 0)
			{
				av_interleaved_write_frame(formatContext, packet);
			}
		}
	}

	//写入结尾
	av_write_trailer(formatContext);

	std::cout << "编码完成" << std::endl;

	//释放资源
	fclose(fp);
	avio_close(formatContext->pb);
	avcodec_free_context(&codecContext);
	avformat_free_context(formatContext);

	return 0;
}