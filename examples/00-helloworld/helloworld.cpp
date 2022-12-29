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
		, m_brush{}
		, m_debug{}
		, m_height{}
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

		m_brush = CharacterCell{ 'a', 0xF, 0x0 };
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
		m_binding[11].set(entry::Key::KeyF, entry::Modifier::None, 1, KeyF, this);
		m_binding[12].set(entry::Key::KeyB, entry::Modifier::None, 1, KeyB, this);
		m_binding[13].set(entry::Key::KeyA, entry::Modifier::None, 1, KeyA, this);
		m_binding[14].end();
		inputAddBindings("Application", m_binding);
	}

	InputBinding m_binding[15];

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
	};
	struct Map
	{
		int m_width;
		int m_height;
		std::vector<CharacterCell> m_vector;
		Map()
		: m_width(0)
		, m_height(0)
		{
		}
		void resize(int width, int height)
		{
			std::vector<CharacterCell> temp = m_vector;
			m_vector.resize(width * height);
			std::fill(m_vector.begin(), m_vector.end(), CharacterCell{ 0x00, 0x0, 0x0 });
			for (int y = 0; y < std::min(height, m_height); ++y)
				for (int x = 0; x < std::min(width, m_width); ++x)
					m_vector[y * width + x] = temp[y * m_width + x];
			m_width = width;
			m_height = height;
		}
		CharacterCell& operator()(int x, int y)
		{
			assert(x >= 0 && x < m_width);
			assert(y >= 0 && y < m_height);
			return m_vector[y * m_width + x];
		}
		const CharacterCell& operator()(int x, int y) const
		{
			assert(x >= 0 && x < m_width);
			assert(y >= 0 && y < m_height);
			return m_vector[y * m_width + x];
		}
	};
	Map m_map;
	Map m_screen;
	int m_x;
	int m_y;
	CharacterCell m_brush;

	static void ShiftLeftBracket(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->ShiftLeftBracket(); }
	static void ShiftRightBracket(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->ShiftRightBracket(); }
	static void LeftBracket(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->LeftBracket(); }
	static void RightBracket(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->RightBracket(); }
	static void Comma(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->Comma(); }
	static void KeyCtrlO(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyCtrlO(); }
	static void KeyCtrlS(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyCtrlS(); }
	static void KeyF(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyF(); }
	static void KeyB(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyB(); }
	static void KeyA(const void* userData) { return static_cast<ExampleHelloWorld*>(const_cast<void*>(userData))->KeyA(); }

	enum Mode
	{
		kForeground,
		kBackground,
		kAscii,
	};
	Mode m_mode;

	void KeyF()
	{
		m_mode = kForeground;
	}

	void KeyB()
	{
		m_mode = kBackground;
	}

	void KeyA()
	{
		m_mode = kAscii;
	}

	void LeftBracket() 
	{
		switch(m_mode)
		{
		case kForeground: m_brush.m_foreground = m_brush.m_foreground - 1; break;
		case kBackground: m_brush.m_background = m_brush.m_background - 1; break;
		case kAscii:      m_brush.m_character = m_brush.m_character - 1; break;
		default: break;
		}
	}

	void ShiftLeftBracket()
	{
		switch (m_mode)
		{
		case kForeground: m_brush.m_foreground = m_brush.m_foreground - 4; break;
		case kBackground: m_brush.m_background = m_brush.m_background - 4; break;
		case kAscii:      m_brush.m_character = m_brush.m_character - 16; break;
		default: break;
		}
	}

	void RightBracket()
	{
		switch (m_mode)
		{
		case kForeground: m_brush.m_foreground = m_brush.m_foreground + 1; break;
		case kBackground: m_brush.m_background = m_brush.m_background + 1; break;
		case kAscii:      m_brush.m_character = m_brush.m_character + 1; break;
		default: break;
		}
	}

	void ShiftRightBracket()
	{
		switch (m_mode)
		{
		case kForeground: m_brush.m_foreground = m_brush.m_foreground + 4; break;
		case kBackground: m_brush.m_background = m_brush.m_background + 4; break;
		case kAscii:      m_brush.m_character = m_brush.m_character + 16; break;
		default: break;
		}
	}

	void Comma()
	{
		m_brush = m_map(m_x, m_y);
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

	static void blit(Map& dest, Map& src, int dx, int dy)
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
			for (int x = 0; x < w; ++x)
				dest(dx + x, dy + y) = src(sx + x, sy + y);
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

			blit(m_screen, m_map, 0, 0);
			blit(m_screen, m_brush, m_x, m_y);
			if (m_mouseState.m_buttons[entry::MouseButton::Left])
			{
				blit(m_map, m_brush, m_x, m_y);
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
