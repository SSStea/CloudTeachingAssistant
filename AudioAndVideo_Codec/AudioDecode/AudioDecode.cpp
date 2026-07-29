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
	//解码aac
	const char inFileName[] = "D:\\0_edoyun\\企业项目实战\\CloudTeachingAssistant\\AudioAndVideo_Codec\\output.aac";
	const char outFileName[] = "output.pcm";

	FILE* file = fopen(outFileName, "wb+");
	if (!file)
	{
		std::cout << "打开输出文件失败！" << std::endl;
		return -1;
	}

	AVFormatContext* formatCtx = nullptr;
	AVCodecContext* codecCtx = nullptr;
	AVPacket* pkt = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();

	int audioStreamIndex = -1;
	do 
	{
		if (avformat_open_input(&formatCtx, inFileName, NULL, NULL) < 0)
		{
			std::cout << "avformat_open_input fail" << std::endl;
			return -2;
		}

		if (avformat_find_stream_info(formatCtx, NULL) < 0)
		{
			std::cout << "avformat_find_stream_info fail" << std::endl;
			return -3;
		}

		for (size_t i = 0; i < formatCtx->nb_streams; i++)
		{
			//如果流信息是音频，更新索引
			if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			{
				audioStreamIndex = i;
				break;
			}
		}

		if (audioStreamIndex == -1)
		{
			std::cout << "not find audio" << std::endl;
			return -4;
		}

		//音频编码参数
		AVCodecParameters* avCodecPara = formatCtx->streams[audioStreamIndex]->codecpar;

		//获取解码器
		const AVCodec* codec = avcodec_find_decoder(avCodecPara->codec_id);
		if (!codec)
		{
			std::cout << "avcodec_find_decoder fail" << std::endl;
			return -5;
		}

		//获取解码上下文
		codecCtx = avcodec_alloc_context3(codec);
		if (!codecCtx)
		{
			std::cout << "avcodec_alloc_context3 fail" << std::endl;
			return -6;
		}

		//打开解码器
		if (avcodec_open2(codecCtx, codec, NULL) < 0)
		{
			std::cout << "avcodec_open2 fail" << std::endl;
			return -7;
		}

		//开始解码
		while (av_read_frame(formatCtx, pkt) >= 0)//有编码数据
		{
			if (pkt->stream_index == audioStreamIndex)//如果是音频数据包
			{
				if (avcodec_send_packet(codecCtx, pkt) >= 0)//发送成功
				{
					//接收解码数据
					while (avcodec_receive_frame(codecCtx, frame) >= 0)
					{
						//接收成功acc，再来判断pcm数据是planer or packed
						if (av_sample_fmt_is_planar(codecCtx->sample_fmt) >= 0)
						{
							int nNumBytes = av_get_bytes_per_sample(codecCtx->sample_fmt);

							for (int i = 0; i < frame->nb_samples; i++)
							{
								for (int ch = 0; ch < codecCtx->ch_layout.nb_channels; ch++)
								{
									fwrite((char*)frame->data[ch] + nNumBytes * i, 1, nNumBytes, file);
								}
							}
						}
					}
				}
			}
			av_packet_unref(pkt);
		}

	} while (0);

	std::cout << "decode done" << std::endl;

	av_frame_free(&frame);
	av_packet_free(&pkt);
	avcodec_free_context(&codecCtx);
	avformat_free_context(formatCtx);
	fclose(file);


	return 0;
}