#pragma warning(disable:4996)
#pragma once
//-------------------------------------------------------------------
//描画用
//-------------------------------------------------------------------
#include "GameEngine_Ver3_83.h"

struct DrawInfo
{
	ML::Box2D draw; //描画サイズと座標。オフセット適用前
	ML::Box2D src; //描画元の画像の範囲
	ML::Color color; ///描画色
};

inline void DrawSprite(const DG::Image::SP img_, const DrawInfo& di_)
{
	if (!img_) { return; }

	img_->Draw(di_.draw, di_.src, di_.color);
}