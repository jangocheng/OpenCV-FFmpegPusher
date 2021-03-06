#include "MediaPusher.h"
#include <iostream>
#include <string>
using namespace std;

extern "C"
{
#include <libavformat/avformat.h>
}
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avformat.lib")


class CMediaPusher :public MediaPusher {
private:
	//封装器
	AVFormatContext *ic = NULL;

	//流地址
	string url = "";

	//视频编码器上下文
	const AVCodecContext *vc = NULL;

	//音频编码器上下文
	const AVCodecContext *ac = NULL;

	//视频流
	AVStream *vs = NULL;

	//音频流
	AVStream *as = NULL;
public:

	void Close()
	{
		if (ic)
		{
			avformat_close_input(&ic);
			vs = NULL;
		}
		vc = NULL;
		url = "";
	}

	bool Init(const char *url) {
		this->url = url;
		///封装器设置
		int result = avformat_alloc_output_context2(&ic, 0, "flv", url);
		if (result < 0)
		{
			char buf[1024] = { 0 };
			av_strerror(result, buf, sizeof(buf) - 1);
			cout << buf;
			return false;
		}
		return true;
	}

	int AddStream(const AVCodecContext *actx) {
		if (!ic)
		{
			return false;
		}
		//添加视频流
		AVStream *vs = avformat_new_stream(ic, NULL);
		if (!vs)
		{
			cout << "avformat_new_stream failed" << endl;
			return false;
		}
		vs->codecpar->codec_tag = 0;

		//从编码器复制参数
		avcodec_parameters_from_context(vs->codecpar, actx);
		av_dump_format(ic, 0, url.c_str(), 1);

		if (actx->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			this->vc = actx;
			this->vs = vs;
		}
		else if (actx->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			this->ac = actx;
			this->as = vs;
		}
		return vs->index;
	}

	bool OpenIO() {
		///打开网络IO流通道
		int result = avio_open(&ic->pb, url.c_str(), AVIO_FLAG_WRITE);
		if (result < 0)
		{
			char buf[1024] = { 0 };
			av_strerror(result, buf, sizeof(buf) - 1);
			cout << buf << endl;
			return false;
		}
		//写入封装头
		result = avformat_write_header(ic, NULL);
		if (result < 0) {
			char buf[1024] = { 0 };
			av_strerror(result, buf, sizeof(buf) - 1);
			cout << buf << endl;
			return false;
		}
		return true;
	}

	bool SendPacket(AVPacket *packet,int streamIndex) {
		if (!packet || packet->size <= 0 || !packet->data)return false;

		AVRational srcTimeBase;
		AVRational desTimeBase;

		packet->stream_index = streamIndex;

		//判断音视频
		if (vs && vc && packet->stream_index == vs->index)
		{
			srcTimeBase = vc->time_base;
			desTimeBase = vs->time_base;
		}
		else if (as && ac && packet->stream_index == as->index)
		{
			srcTimeBase = ac->time_base;
			desTimeBase = as->time_base;
		}
		else
		{
			return false;
		}
		packet->pts = av_rescale_q(packet->pts, srcTimeBase, desTimeBase);
		packet->dts = av_rescale_q(packet->dts, srcTimeBase, desTimeBase);
		packet->duration = av_rescale_q(packet->duration, srcTimeBase, desTimeBase);
		cout << "#size=" << packet->size << flush;
		int result = av_interleaved_write_frame(ic, packet);
		if (result == 0)
		{
			return true;
		}
		return false;
	}
};

MediaPusher * MediaPusher::Get(unsigned char index) {
	static CMediaPusher pusher[255];
	static bool isFirst = true;
	if (isFirst)
	{
		av_register_all();
		avformat_network_init();
		isFirst = false;
	}
	return &pusher[index];
}

MediaPusher::MediaPusher() {

}

MediaPusher::~MediaPusher() {

}