/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Contributor(s): Wander Lairson Costa
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file ghost/intern/GHOST_WindowWayland.h
 *  \ingroup GHOST
 * Declaration of GHOST_WindowWayland class.
 */

#ifndef __GHOST_WINDOWWAYLAND_H__
#define __GHOST_WINDOWWAYLAND_H__

#include "GHOST_Window.h"
#include "GHOST_SystemWayland.h"
#include "wayland_util.h"
#include "wayland_listeners.h"

extern "C" {
#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
}

#include <string>
#include <list>

class STR_String;
class GHOST_SystemWayland;
class GHOST_WindowWayland;

class WindowCallbackBase : public wl::callback_listener {
public:
	WindowCallbackBase(GHOST_WindowWayland *window);
	virtual ~WindowCallbackBase();

	void setup();

protected:
	GHOST_WindowWayland *m_window;

private:
	/*
	 * This member function configures the callback object.
	 */
	virtual wl_callback *configure_callback() = 0;

	/*
	 * This function is called when the wl_back 'done' event arrives.
	 */
	virtual void on_done(struct wl_callback *callback, uint32_t serial) = 0;

	void done(struct wl_callback *callback, uint32_t serial);

private:
	std::list<wl_callback *> m_cb_list;
};

class GHOST_WindowWayland
	: public GHOST_Window
	, public wl::shell_surface_listener
{
public:

	const GHOST_TabletData *GetTabletData() {
		return NULL;
	}

	GHOST_WindowWayland(GHOST_SystemWayland *system,
	                const STR_String& title,
	                GHOST_TInt32 left, GHOST_TInt32 top,
	                GHOST_TUns32 width, GHOST_TUns32 height,
	                GHOST_TWindowState state,
	                const GHOST_TEmbedderWindowID parentWindow,
	                GHOST_TDrawingContextType type = GHOST_kDrawingContextTypeNone,
	                const bool stereoVisual = false,
	                const bool exclusive = false,
	                const GHOST_TUns16 numOfAASamples = 0
	                );

	~GHOST_WindowWayland();

	GHOST_TSuccess invalidate(void);

	bool getValid() const
	{
		return true;
	}

	void getWindowBounds(GHOST_Rect& bounds) const;
	void getClientBounds(GHOST_Rect& bounds) const;

protected:
	GHOST_TSuccess installDrawingContext(GHOST_TDrawingContextType type);
	GHOST_TSuccess removeDrawingContext();

	GHOST_TSuccess
	setWindowCursorGrab(GHOST_TGrabCursorMode mode);

	GHOST_TSuccess
	setWindowCursorShape(GHOST_TStandardCursor shape);

	GHOST_TSuccess
	setWindowCustomCursorShape(GHOST_TUns8 bitmap[16][2],
	                           GHOST_TUns8 mask[16][2],
	                           int hotX, int hotY);

	GHOST_TSuccess
	setWindowCustomCursorShape(GHOST_TUns8 *bitmap,
	                           GHOST_TUns8 *mask,
	                           int sizex, int sizey,
	                           int hotX, int hotY,
	                           int fg_color, int bg_color);

	GHOST_TSuccess
	setWindowCursorVisibility(bool visible);

	void
	setTitle(const STR_String& title);

	void
	getTitle(STR_String& title) const;

	GHOST_TSuccess
	setClientWidth(GHOST_TUns32 width);

	GHOST_TSuccess
	setClientHeight(GHOST_TUns32 height);

	GHOST_TSuccess
	setClientSize(GHOST_TUns32 width,
	              GHOST_TUns32 height);

	void
	screenToClient(GHOST_TInt32 inX, GHOST_TInt32 inY,
	               GHOST_TInt32& outX, GHOST_TInt32& outY) const;

	void
	clientToScreen(GHOST_TInt32 inX, GHOST_TInt32 inY,
	               GHOST_TInt32& outX, GHOST_TInt32& outY) const;

	GHOST_TSuccess
	swapBuffers();

	GHOST_TSuccess
	activateDrawingContext();

	GHOST_TSuccess
	setState(GHOST_TWindowState state);

	GHOST_TWindowState
	getState() const;

	GHOST_TSuccess setOrder(GHOST_TWindowOrder order)
	{
		return GHOST_kSuccess;
	}

	GHOST_TSuccess beginFullScreen() const;

	GHOST_TSuccess endFullScreen() const;

private:

	class WindowDrawHandler : public WindowCallbackBase {
	public:
		WindowDrawHandler(GHOST_WindowWayland *window)
			: WindowCallbackBase(window)
		{ }

	private:
		void on_done(struct wl_callback *callback, uint32_t serial);
		wl_callback *configure_callback();
	};

	class DisplaySyncHandler : public WindowCallbackBase {
	public:
		DisplaySyncHandler(GHOST_WindowWayland *window)
			: WindowCallbackBase(window)
		{ }

	private:
		void on_done(struct wl_callback *callback, uint32_t serial);
		wl_callback *configure_callback();
	};

private:

	void context_make_current(EGLSurface surf);

	void resize(void) const;

	virtual void ping(
		struct wl_shell_surface *shell_surface,
		uint32_t serial);

	virtual void configure(
		struct wl_shell_surface *shell_surface,
		uint32_t edges,
		int32_t width,
		int32_t height);

private:

	GHOST_SystemWayland  *m_system;
	bool m_invalid_window;

	wl_surface *m_surface;
	wl_shell_surface *m_shell_surface;
	wl_egl_window *m_window;
	EGLSurface m_egl_surface;
	EGLContext m_egl_context;

	std::string m_title;
	int m_x;
	int m_y;

	int m_width;
	int m_height;

	/*
	 * When we go fullscreen, we have to backup the window size
	 * to restore it when we come back to the normal mode.
	 */
	mutable int m_nfs_width;
	mutable int m_nfs_height;

	GHOST_TWindowState m_state;
	WindowDrawHandler m_redraw;
	mutable DisplaySyncHandler m_sync;
};

#endif // __GHOST_WINDOWWAYLAND_H__
