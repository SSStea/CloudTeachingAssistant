#include <iostream>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/avutil.h"
}

int main()
{
	//解码h264
	const char inFileName[] = "D:\\0_edoyun\\output.h264";
	const char outFileName[] = "output.yuv";

	//准备一个封装上下文
	AVFormatContext* fmtCtx = avformat_alloc_context();

	//打开输入文件
	if (avformat_open_input(&fmtCtx, inFileName, NULL, NULL) != 0)
	{
		std::cout << "avformat_open_input fail" << std::endl;
		return -1;
	}

	//获取流信息
	if (avformat_find_stream_info(fmtCtx, NULL) < 0)
	{
		std::cout << "avformat_find_stream_info fail" << std::endl;
		return -2;
	}

	//查找视频流信息
	int videoStreamIndex = -1;
	//遍历流
	for (int i = 0; i < fmtCtx->nb_streams; i++)
	{
		//如果是视频流
		if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStreamIndex = i;
			break;
		}
	}

	if (videoStreamIndex == -1)
	{
		std::cout << "not find vedio stream info" << std::endl;
		return -3;
	}

	//获取视频解码器
	AVCodecParameters* codecPara = fmtCtx->streams[videoStreamIndex]->codecpar;
	const AVCodec* codec = avcodec_find_decoder(codecPara->codec_id);
	if (!codec)
	{
		std::cout << "avcodec_find_decoder fail" << std::endl;
		return -4;
	}

	//分配解码器上下文
	AVCodecContext* codecCtx = avcodec_alloc_context3(codec);
	if (!codecCtx)
	{
		std::cout << "avcodec_alloc_context3 fail" << std::endl;
		return -5;
	}

	//将解码器参数拷贝
	avcodec_parameters_to_context(codecCtx, codecPara);

	//打开解码器
	if (avcodec_open2(codecCtx, codec, NULL) < 0)
	{
		std::cout << "avcodec_open2 fail" << std::endl;
		return -6;
	}

	//打开输出文件
	FILE* file = fopen(outFileName, "wb");
	if (!file)
	{
		std::cout << "open output file fail" << std::endl;
		return -7;
	}

	//解码
	AVFrame* frame = av_frame_alloc();
	AVPacket* pkt = av_packet_alloc();

	int nRet = 0;
	while (av_read_frame(fmtCtx, pkt) >= 0)
	{
		if (pkt->stream_index == videoStreamIndex)//是视频流
		{
			//开始发送数据包解码
			nRet = avcodec_send_packet(codecCtx, pkt);
			if (nRet < 0)
			{
				std::cout << "send fail" << std::endl;
				break;
			}
			while (nRet >= 0)
			{
				nRet = avcodec_receive_frame(codecCtx, frame);
				if (nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF)
				{
					break;
				}
				else if (nRet < 0)
				{
					std::cout << "decode fail" << std::endl;
					break;
				}
				//解码成功，需要写入yuv数据，ffmpeg默认解码就是yuv240p
				for (int i = 0; i < frame->height; i++)//y
				{
					fwrite(frame->data[0] + i * frame->linesize[0], 1, frame->width, file);
				}
				for (int i = 0; i < frame->height / 2; i++)//u
				{
					fwrite(frame->data[1] + i * frame->linesize[1], 1, frame->width / 2, file);
				}
				for (int i = 0; i < frame->height / 2; i++)//v
				{
					fwrite(frame->data[2] + i * frame->linesize[2], 1, frame->width / 2, file);
				}
			}
		}
		av_packet_unref(pkt);
	}

	std::cout << "decode done" << std::endl;

	fclose(file);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	avcodec_free_context(&codecCtx);
	avformat_free_context(fmtCtx);

	return -1;
}