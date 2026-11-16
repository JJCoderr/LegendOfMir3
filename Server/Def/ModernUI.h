// =============================================================================
//  Server/Def/ModernUI.h  —— Win32 程序"现代外观"辅助函数
//
//  与根目录 app.manifest（Common Controls 6.0 主题外观）配合使用：
//      * 控件由系统主题绘制（清单在各程序自己的 *.rc 里以 RT_MANIFEST 嵌入）
//      * 窗口尺寸在这里按 DPI 换算，不再依赖 CW_USEDEFAULT 给出的超大默认值
//
//  所有 Server 端程序都链接 Def 静态库，因此只需在这里实现一次。
// =============================================================================
#ifndef _MIR3_MODERN_UI_H
#define _MIR3_MODERN_UI_H

#pragma once

#include <windows.h>

// 96 DPI 下的逻辑像素 -> 当前窗口所在显示器的实际像素
int Mir3DpiScale(HWND hWnd, int nLogical);

// 按逻辑尺寸（96 DPI 下的像素）设置主窗口大小：
//   * 按显示器 DPI 放大
//   * 限制在当前显示器工作区内（留边距）
//   * 在工作区居中
// 调用时机：CreateWindowEx 之后、按 GetClientRect 布局子控件之前。
void Mir3SizeMainWindow(HWND hWnd, int nLogicalWidth, int nLogicalHeight);

	// 日志列表列宽统一：按列数平均分配（各列尽量一致），最后一列吃掉余数，
	// 正好铺满列表宽度；窗口缩放或 DPI 变化后各程序的比例保持一致。
	// hList：列表控件；hWndForDpi：取 DPI 用的窗口（一般传主窗口 / 对话框）。
void Mir3FitLogColumns(HWND hList, HWND hWndForDpi);

#endif // _MIR3_MODERN_UI_H