/*
 * Copyright 2011-2022 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "logo.h"
#include "imgui/imgui.h"
#include <vector>
#include "entry/input.h"
#include "entry/dialog.h"
#include <bx/file.h>


namespace
{

class ExampleHelloWorld : public entry::AppI
{
public:
	ExampleHelloWorld(const char* _name, const char* _description, const char* _url)
		: entry::AppI(_name, _description, _url)
		, m_binding()
		, m_debug{}
		, m_height{}
		, m_foreground{}
		, m_mode{}
		, m_reset{}
		, m_x{}
		, m_y{}
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_TEXT;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.platformData.nwh  = entry::getNativeWindowHandle(entry::kDefaultWindowHandle);
		init.platformData.ndt  = entry::getNativeDisplayHandle();
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
			);

		imguiCreate();

		m_foreground = CharacterCell{ 'a', 0xF, 0x0 };
		m_binding[0].set(entry::Key::LeftBracket, entry::Modifier::None, 1, LeftBracket, this);
		m_binding[1].set(entry::Key::RightBracket, entry::Modifier::None, 1, RightBracket, this);
		m_binding[2].set(entry::Key::LeftBracket, entry::Modifier::LeftShift, 1, ShiftLeftBracket, this);
		m_binding[3].set(entry::Key::LeftBracket, entry::Modifier::RightShift, 1, ShiftLeftBracket, this);
		m_binding[4].set(entry::Key::RightBracket, entry::Modifier::LeftShift, 1, ShiftRightBracket, this);
		m_binding[5].set(entry::Key::RightBracket, entry::Modifier::RightShift, 1, ShiftRightBracket, this);
		m_binding[6].set(entry::Key::Comma, entry::Modifier::None, 1, Comma, this);
		m_binding[7].set(entry::Key::KeyO, entry::Modifier::LeftCtrl, 1, KeyCtrlO, this);
		m_binding[8].set(entry::Key::KeyO, entry::Modifier::RightCtrl, 1, KeyCtrlO, this);
		m_binding[9].set(entry::Key::KeyS, entry::Modifier::LeftCtrl, 1, KeyCtrlS, this);
		m_binding[10].set(entry::Key::KeyS, entry::Modifier::RightCtrl, 1, KeyCtrlS, this);
		m_binding[11].set(entry::Key::Key1, entry::Modifier::None, 1, Key1, this);
		m_binding[12].set(entry::Key::Key2, entry::Modifier::None, 1, Key2, this);
		m_binding[13].set(entry::Key::Key3, entry::Modifier::None, 1, Key3, this);
		m_binding[14].set(entry::Key::KeyB, entry::Modifier::None, 1, KeyB, this);
		m_binding[15].set(entry::Key::KeyP, entry::Modifier::None, 1, KeyP, this);
		m_binding[16].set(entry::Key::F1, entry::Modifier::None, 1, F1, this);
		m_binding[17].set(entry::Key::F2, entry::Modifier::None, 1, F2, this);
		m_binding[18].set(entry::Key::KeyX, entry::Modifier::None,      1,      KeyX, this);
		m_binding[19].set(entry::Key::KeyX, entry::Modifier::LeftShift, 1, ShiftKeyX, this);
		m_binding[20].set(entry::Key::KeyY, entry::Modifier::None,      1,      KeyY, this);
		m_binding[21].set(entry::Key::KeyY, entry::Modifier::LeftShift, 1, ShiftKeyY, this);
		m_binding[22].set(entry::Key::KeyH, entry::Modifier::None,      1,      KeyH, this);
		m_binding[23].set(entry::Key::KeyH, entry::Modifier::LeftShift, 1, ShiftKeyH, this);
		m_binding[24].set(entry::Key::Minus, entry::Modifier::None, 1, Minus, this);
		m_binding[25].set(entry::Key::Plus, entry::Modifier::None, 1, Plus, this);
		m_binding[26].end();
		inputAddBindings("Application", m_binding);
	}

	InputBinding m_binding[27];

	virtual int shutdown() override
	{
		imguiDestroy();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	struct CharacterCell
	{
		uint8_t m_character;
		uint8_t m_foreground:4;
		uint8_t m_background:4;
		friend bool operator==(const CharacterCell& a, const CharacterCell& b) 
		{
			if (a.m_character != b.m_character)
				return false;
			if (a.m_foreground != b.m_foreground)
				return false;
			if (a.m_background != b.m_background)
				return false;
			return true;
		}
		friend bool operator!=(const CharacterCell& a, const CharacterCell& b)
		{
			return !(a == b);
		}
	};
	template <typename T> struct MapOf
	{
		int m_width;
		int m_height;
		std::vector<T> m_vector;
		MapOf()
		: m_width(0)
		, m_height(0)
		{
		}
		MapOf& operator=(const MapOf& other)
		{
			m_vector = other.m_vector;
			m_width = other.m_width;
			m_height = other.m_height;
			return *this;
		}
		MapOf& operator=(MapOf&& other)
		{
			m_vector = std::move(other.m_vector);
			m_width = other.m_width;
			other.m_width = 0;
			m_height = other.m_height;
			other.m_height = 0;
			return *this;
		}
		void resize(int width, int height)
		{
			std::vector<T> temp = m_vector;
			m_vector.resize(width * height);
			std::fill(m_vector.begin(), m_vector.end(), CharacterCell{ 0x00, 0x0, 0x0 });
			for (int y = 0; y < std::min(height, m_height); ++y)
				for (int x = 0; x < std::min(width, m_width); ++x)
					m_vector[y * width + x] = temp[y * m_width + x];
			m_width = width;
			m_height = height;
		}
		T& operator()(int x, int y)
		{
			assert(x >= 0 && x < m_width);
			assert(y >= 0 && y < m_height);
			return m_vector[y * m_width + x];
		}
		const T& operator()(int x, int y) const
		{
			assert(x >= 0 && x < m_width);
			assert(y >= 0 && y < m_height);
			return m_vector[y * m_width + x];
		}
	};
	typedef MapOf<CharacterCell> Map;
	Map m_map = {};
	Map m_screen = {};
	int m_x = 0;
	int m_y = 0;
	int m_brushx = 0;
	int m_brushy = 0;
	CharacterCell m_foreground = {};
	Map m_oldbrush = {};
	Map m_brush = {};
	Map m_unscaledBrush = {};
	CharacterCell m_brushColor = {};

	static void ShiftLeftBracket(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->ShiftLeftBracket(); }
	static void ShiftRightBracket(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->ShiftRightBracket(); }
	static void LeftBracket(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->LeftBracket(); }
	static void RightBracket(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->RightBracket(); }
	static void Comma(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->Comma(); }
	static void KeyCtrlO(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyCtrlO(); }
	static void KeyCtrlS(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyCtrlS(); }
	static void Key1(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->Key1(); }
	static void Key2(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->Key2(); }
	static void Key3(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->Key3(); }
	static void KeyB(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyB(); }
	static void KeyP(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyP(); }
	static void F1(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->F1(); }
	static void F2(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->F2(); }

	static void KeyH(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyH(); }
	static void ShiftKeyH(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->ShiftKeyH(); }
	static void KeyX(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyX(); }
	static void ShiftKeyX(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->ShiftKeyX(); }
	static void KeyY(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyY(); }
	static void ShiftKeyY(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->ShiftKeyY(); }

	static void Minus(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->Minus(); }
	static void Plus(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->Plus(); }

	enum Tool
	{
		kPaint,
		kGrabBrush
	};
	Tool m_tool = kPaint;

	enum Mode
	{
		kForeground,
		kBackground,
		kAscii,
	};
	Mode m_mode = kForeground;

	void F1()
	{
		m_blitMode = kMatte;
	}

	void F2()
	{
		m_blitMode = kColor;
	}

	void Key1()
	{
		m_mode = kForeground;
	}

	void Key2()
	{
		m_mode = kBackground;
	}

	void Key3()
	{
		m_mode = kAscii;
	}

	void KeyB()
	{
		m_tool = kGrabBrush;
		m_oldX = m_x;
		m_oldY = m_y;
	}

	void KeyP()
	{
		m_tool = kPaint;
	}

	void Minus()
	{
		const int w1 = m_brush.m_width - 1;
		const int h1 = m_unscaledBrush.m_height * w1 / m_unscaledBrush.m_width;
		const int h2 = m_brush.m_height - 1;
		const int w2 = m_unscaledBrush.m_width * h2 / m_unscaledBrush.m_height;
		if(w2*h2 > w1*h1)
			AdjustBrushSize(w2, h2);
		else
			AdjustBrushSize(w1, h1);
	}
	void Plus()
	{
		const int w1 = m_brush.m_width + 1;
		const int h1 = m_unscaledBrush.m_height * w1 / m_unscaledBrush.m_width;
		const int h2 = m_brush.m_height + 1;
		const int w2 = m_unscaledBrush.m_width * h2 / m_unscaledBrush.m_height;
		if (w2 * h2 < w1 * h1)
			AdjustBrushSize(w2, h2);
		else
			AdjustBrushSize(w1, h1);
	}

	void KeyH()
	{
		AdjustBrushSize(m_brush.m_width / 2, m_brush.m_height / 2);
	}
	void ShiftKeyH()
	{
		AdjustBrushSize(m_brush.m_width * 2, m_brush.m_height * 2);
	}
	void KeyX()
	{
		FlipHorizontal();
	}
	void ShiftKeyX()
	{
		AdjustBrushSize(m_brush.m_width * 2, m_brush.m_height);
	}
	void KeyY()
	{
		FlipVertical();
	}
	void ShiftKeyY()
	{
		AdjustBrushSize(m_brush.m_width, m_brush.m_height * 2);
	}

	void LeftBracket() 
	{
		switch(m_mode)
		{
		case kForeground: m_foreground.m_foreground = m_foreground.m_foreground - 1; break;
		case kBackground: m_foreground.m_background = m_foreground.m_background - 1; break;
		case kAscii:      m_foreground.m_character = m_foreground.m_character - 1; break;
		default: break;
		}
	}

	void ShiftLeftBracket()
	{
		switch (m_mode)
		{
		case kForeground: m_foreground.m_foreground = m_foreground.m_foreground - 4; break;
		case kBackground: m_foreground.m_background = m_foreground.m_background - 4; break;
		case kAscii:      m_foreground.m_character = m_foreground.m_character - 16; break;
		default: break;
		}
	}

	void RightBracket()
	{
		switch (m_mode)
		{
		case kForeground: m_foreground.m_foreground = m_foreground.m_foreground + 1; break;
		case kBackground: m_foreground.m_background = m_foreground.m_background + 1; break;
		case kAscii:      m_foreground.m_character = m_foreground.m_character + 1; break;
		default: break;
		}
	}

	void ShiftRightBracket()
	{
		switch (m_mode)
		{
		case kForeground: m_foreground.m_foreground = m_foreground.m_foreground + 4; break;
		case kBackground: m_foreground.m_background = m_foreground.m_background + 4; break;
		case kAscii:      m_foreground.m_character = m_foreground.m_character + 16; break;
		default: break;
		}
	}

	static void grab(Map& dest, Map& src, int x, int y, int width, int height)
	{
		dest.resize(width, height);
		for (int dy = 0; dy < height; ++dy)
			for (int dx = 0; dx < width; ++dx)
				dest(dx, dy) = src(x + dx, y + dy);
	}

	void Comma()
	{
		m_foreground = m_map(m_x, m_y);
		grab(m_brush, m_map, m_x, m_y, 1, 1);
	}

	void KeyCtrlO()
	{
		bx::FilePath filePath = {};
		bx::FileReader reader = {};
		bx::Error err = {};
		openFileSelectionDialog(filePath, FileSelectionDialogType::Open, "open file");
		if (bx::open(&reader, filePath, &err))
		{
			int width, height;
			reader.read(&width, sizeof(width), &err);
			reader.read(&height, sizeof(height), &err);
			m_map.resize(width, height);
			reader.read(&m_map.m_vector[0], sizeof(CharacterCell) * m_map.m_width * m_map.m_height, &err);
			bx::close(&reader);
		}
	}

	void KeyCtrlS()
	{
		bx::FilePath filePath = {};
		bx::FileWriter writer = {};
		bx::Error err = {};
		openFileSelectionDialog(filePath, FileSelectionDialogType::Save, "save file");
		if (bx::open(&writer, filePath, false, &err))
		{
			writer.write(&m_map.m_width, sizeof(m_map.m_width), &err);
			writer.write(&m_map.m_height, sizeof(m_map.m_height), &err);
			writer.write(&m_map.m_vector[0], sizeof(CharacterCell) * m_map.m_width * m_map.m_height, &err);
			bx::close(&writer);
		}
	}

	static void xor(Map& dest, int x, int y)
	{
		CharacterCell cell = dest(x, y);
		cell.m_background ^= 0xF;
		cell.m_foreground ^= 0xF;
		cell.m_character ^= 0xFF;
		dest(x, y) = cell;
	}

	static void ants(Map& dest, int x1, int y1, int x2, int y2, int frame)
	{
		frame &= 7;
		int f = 0;
		int x = x1;
		int y = y1;
		for (; x < x2; ++x)
		{
			if ((f & 7) == frame)
				dest(x, y) = CharacterCell{ 0,0xf,0xf };
			++f;
		}
		for (; y < y2; ++y)
		{
			if ((f & 7) == frame)
				dest(x, y) = CharacterCell{ 0,0xf,0xf};
			++f;
		}
		for (; x > x1; --x)
		{
			if ((f & 7) == frame)
				dest(x, y) = CharacterCell{ 0,0xf,0xf };
			++f;
		}
		for (; y > y1; --y)
		{
			if ((f & 7) == frame)
				dest(x, y) = CharacterCell{ 0,0xf,0xf };
			++f;
		}
	}

	enum BlitMode
	{
		kMatte,    // with a transparent color
		kColor,    // as a solid color
		kReplace,  // ?
	};
	BlitMode m_blitMode = kMatte;

	void FlipHorizontal()
	{
		for(int y = 0; y < m_unscaledBrush.m_height; ++y)
			for (int x = 0; x < m_unscaledBrush.m_width / 2; ++x)
				std::swap(m_unscaledBrush(x,y), m_unscaledBrush(m_unscaledBrush.m_width - 1 - x, y));
		RescaleBrush();
	}

	void FlipVertical()
	{
		for (int y = 0; y < m_unscaledBrush.m_height / 2; ++y)
			for (int x = 0; x < m_unscaledBrush.m_width; ++x)
				std::swap(m_unscaledBrush(x, y), m_unscaledBrush(x, m_unscaledBrush.m_height - 1 - y));
		RescaleBrush();
	}

	void RescaleBrush()
	{
		const int dsxddx = (m_unscaledBrush.m_width << 16) / m_brush.m_width;
		const int dsyddy = (m_unscaledBrush.m_height << 16) / m_brush.m_height;
		int sy = 0;
		for (int dy = 0; dy < m_brush.m_height; ++dy)
		{
			int sx = 0;
			for (int dx = 0; dx < m_brush.m_width; ++dx)
			{
				const auto isx = sx >> 16;
				const auto isy = sy >> 16;
				m_brush(dx, dy) = m_unscaledBrush(isx, isy);
				sx += dsxddx;
			}
			sy += dsyddy;
		}
	}

	void AdjustBrushSize(int width, int height)
	{
		m_brush.resize(width, height);
		RescaleBrush();
	}

	void blit(Map& dest, Map& src, int dx, int dy, BlitMode blitMode)
	{
		int sx = 0;
		int sy = 0;
		if (dx < 0)
		{
			sx += -dx;
			dx = 0;
		}
		if (dy < 0)
		{
			sy += -dy;
			dy = 0;
		}
		int w = std::max(0, std::min(src.m_width - sx, dest.m_width - dx));
		int h = std::max(0, std::min(src.m_height - sy, dest.m_height - dy));
		for (int y = 0; y < h; ++y)
			switch (blitMode)
			{
			case kMatte:
				for (int x = 0; x < w; ++x)
				{
					const CharacterCell cell = src(sx + x, sy + y);
					if (cell != m_brushColor)
						dest(dx + x, dy + y) = cell;
				}
				break;
			case kColor:
				for (int x = 0; x < w; ++x)
				{
					const CharacterCell cell = src(sx + x, sy + y);
					if (cell != m_brushColor)
						dest(dx + x, dy + y) = m_foreground;
				}
				break;
			case kReplace:
				for (int x = 0; x < w; ++x)
				{
					dest(dx + x, dy + y) = src(sx + x, sy + y);
				}
				break;
			default:
				break;
			}
	}

	void blit(Map& dest, Map& src, int dx, int dy)
	{
		blit(dest, src, dx, dy, m_blitMode);
	}

	static void blit(Map& dest, CharacterCell& src, int dx, int dy)
	{
		if (dx < 0)
			return;
		if (dy < 0)
			return;
		if (dx >= dest.m_width)
			return;
		if (dy >= dest.m_height)
			return;
		dest(dx, dy) = src;
	}

	void UpdatePaint()
	{
		blit(m_screen, m_brush, m_x - m_brushx, m_y - m_brushy);
		if (m_mouseState.m_buttons[entry::MouseButton::Left])
		{
			blit(m_map, m_brush, m_x - m_brushx, m_y - m_brushy);
		}
	}

	bool m_grabbing = false;
	int m_oldX, m_oldY;
	void UpdateGrabBrush()
	{
		const int x1 = std::min(m_x, m_oldX);
		const int y1 = std::min(m_y, m_oldY);
		const int x2 = std::max(m_x, m_oldX);
		const int y2 = std::max(m_y, m_oldY);
		const int width = x2 - x1 + 1;
		const int height = y2 - y1 + 1;
		if (m_grabbing)
		{
			if (!m_mouseState.m_buttons[entry::MouseButton::Left])
			{
				const auto corner0 = m_map(x1, y1);
				const auto corner1 = m_map(x2, y1);
				const auto corner2 = m_map(x1, y2);
				const auto corner3 = m_map(x2, y2);
				if (corner0 == corner1 && corner1 == corner2 && corner2 == corner3)
					m_brushColor = corner0;
				else
					m_brushColor = m_foreground;
				grab(m_unscaledBrush, m_map, x1, y1, width, height);
				m_brush = m_unscaledBrush;
				m_brushx = width / 2;
				m_brushy = height / 2;
				m_grabbing = false;
				m_tool = kPaint;
			}
		}
		else
		{
			if (m_mouseState.m_buttons[entry::MouseButton::Left])
			{
				m_grabbing = true;
			}
			m_oldX = m_x;
			m_oldY = m_y;
		}
		ants(m_screen, x1, y1, x2, y2, m_frame++);
	}
	int m_frame = 0;

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);

			imguiEndFrame();

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			const bgfx::Stats* stats = bgfx::getStats();
			m_screen.resize(stats->textWidth, stats->textHeight);
			m_map.resize(stats->textWidth, stats->textHeight);

//			uint8_t modifiers;
			// Use debug font to print information about this example.
			bgfx::dbgTextClear();

			const int characterWideInPixels = stats->width / stats->textWidth;
			const int characterTallInPixels = stats->height / stats->textHeight;
			m_x = (m_mouseState.m_mx + (characterWideInPixels / 2)) / characterWideInPixels;
			m_y = (m_mouseState.m_my + (characterTallInPixels / 2)) / characterTallInPixels;

			blit(m_screen, m_map, 0, 0, kReplace);
			switch (m_tool)
			{
			case kPaint:
				UpdatePaint();
				break;
			case kGrabBrush:
				UpdateGrabBrush();
				break;
			}
			bgfx::dbgTextImage(0, 0, stats->textWidth, stats->textHeight, &m_screen.m_vector[0], stats->textWidth * sizeof(CharacterCell));

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	  ExampleHelloWorld
	, "00-helloworld"
	, "Initialization and debug text."
	, "https://bkaradzic.github.io/bgfx/examples.html#helloworld"
	);
