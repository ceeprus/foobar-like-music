#include "stdafx.h"

DECLARE_COMPONENT_VERSION(
	"Like Heart",
	"1.0.0",
	"Adds a clickable \"Like\" column to the Default UI playlist view.\n"
	"Click the heart to like or unlike a track. A like is a Playback Statistics rating.\n"
	"Also adds a \"Toggle Like\" command for keyboard shortcuts."
);

VALIDATE_COMPONENT_FILENAME("foo_likeheart.dll");

namespace {
	static constexpr GUID guid_column = { 0x6f1c2a44, 0x8b0e, 0x4d5b, { 0x9a, 0x3e, 0x52, 0x7d, 0x41, 0xc8, 0x0b, 0x19 } };
	static constexpr GUID guid_toggle = { 0x2d9e7f31, 0x4c6a, 0x4e0b, { 0xb1, 0x58, 0x0e, 0x6a, 0x93, 0x27, 0xd4, 0x8c } };

	static constexpr wchar_t columnTitle[] = L"Like";
	static constexpr char columnTitleUtf8[] = "Like";

	// U+2665 / U+2661 share one size in Segoe UI; U+2764 is missing there and falls back to a bigger glyph.
	// No $rgb(): the Default UI playlist prints color codes as literal text.
	static constexpr char columnPattern[] = "$if(%rating%,\xE2\x99\xA5,\xE2\x99\xA1)";

	// Playback Statistics context menu entries. Matched case-insensitively, ignoring '&'.
	static const char * const pathLike[] = { "Playback Statistics", "Rating", "5" };
	static const char * const pathUnlike[] = { "Playback Statistics", "Rating", "<not set>" };

	pfc::string8 cleanName(const char * name) {
		pfc::string8 out;
		for (const char * p = name; *p; ++p) if (*p != '&') out.add_byte(*p);
		return out;
	}

	contextmenu_node * findChild(contextmenu_node * parent, const char * name) {
		if (parent == nullptr || parent->get_type() != contextmenu_item_node::TYPE_POPUP) return nullptr;
		for (size_t i = 0; i < parent->get_num_children(); ++i) {
			contextmenu_node * child = parent->get_child(i);
			if (child != nullptr && pfc::stricmp_ascii(cleanName(child->get_name()), name) == 0) return child;
		}
		return nullptr;
	}

	void dumpTree(contextmenu_node * node, unsigned depth) {
		if (node == nullptr) return;
		pfc::string8 line;
		for (unsigned i = 0; i < depth; ++i) line << "  ";
		line << node->get_name();
		console::print(line);
		if (node->get_type() == contextmenu_item_node::TYPE_POPUP) {
			for (size_t i = 0; i < node->get_num_children(); ++i) dumpTree(node->get_child(i), depth + 1);
		}
	}

	template<size_t N>
	bool runContextCommand(metadb_handle_list_cref items, const char * const (&path)[N]) {
		auto manager = contextmenu_manager::get();
		manager->init_context(items, contextmenu_manager::flag_view_full);
		contextmenu_node * node = manager->get_root();
		for (const char * name : path) node = findChild(node, name);
		if (node != nullptr && node->get_type() == contextmenu_item_node::TYPE_COMMAND) {
			node->execute();
			return true;
		}
		if (findChild(manager->get_root(), path[0]) == nullptr) {
			console::print("Like Heart: install the Playback Statistics component (foo_playcount); likes are stored as its ratings.");
			return false;
		}
		console::print("Like Heart: could not find Playback Statistics > Rating in the context menu. Menu tree:");
		dumpTree(manager->get_root(), 1);
		return false;
	}

	bool isLiked(const metadb_handle_ptr & track) {
		static titleformat_object::ptr script;
		if (script.is_empty()) titleformat_compiler::get()->compile_force(script, "%rating%");
		pfc::string8 rating;
		track->format_title(nullptr, rating, script, nullptr);
		return !rating.is_empty() && rating != "?";
	}

	void toggleLike(metadb_handle_list_cref items) {
		if (items.get_count() == 0) return;
		// Mixed selections follow the first track, so one press makes them all match.
		if (isLiked(items[0])) runContextCommand(items, pathUnlike);
		else runContextCommand(items, pathLike);
	}

	void toggleSelectionOrPlaying() {
		auto playlists = playlist_manager::get();
		metadb_handle_list items;
		playlists->activeplaylist_get_selected_items(items);
		if (items.get_count() == 0) {
			metadb_handle_ptr playing;
			if (playback_control::get()->get_now_playing(playing)) items.add_item(playing);
		}
		toggleLike(items);
	}

	// Called after the playlist view has handled the click, so the clicked row is the focused, sole selection.
	void toggleClickedRow() {
		auto playlists = playlist_manager::get();
		const size_t playlist = playlists->get_active_playlist();
		if (playlist == SIZE_MAX) return;
		const size_t focus = playlists->playlist_get_focus_item(playlist);
		if (focus == SIZE_MAX || !playlists->playlist_is_item_selected(playlist, focus)) return;
		if (playlists->playlist_get_selection_count(playlist, 2) != 1) return;
		metadb_handle_ptr track;
		if (!playlists->playlist_get_item_handle(track, playlist, focus)) return;
		metadb_handle_list items;
		items.add_item(track);
		toggleLike(items);
	}

	class likeColumn : public fb2k::playlistColumnProvider {
	public:
		size_t numColumns() override { return 1; }
		GUID columnID(size_t) override { return guid_column; }
		fb2k::stringRef columnFormatSpec(size_t) override { return fb2k::makeString(columnPattern); }
		fb2k::stringRef columnSortScript(size_t) override { return fb2k::makeString("$if(%rating%,0,1)"); }
		fb2k::stringRef columnName(size_t) override { return fb2k::makeString(columnTitleUtf8); }
		unsigned columnFlags(size_t) override { return flag_alignCenter; }
	};
	FB2K_SERVICE_FACTORY(likeColumn);

	class likeCommand : public mainmenu_commands {
	public:
		uint32_t get_command_count() override { return 1; }
		GUID get_command(uint32_t) override { return guid_toggle; }
		void get_name(uint32_t, pfc::string_base & out) override { out = "Toggle Like"; }
		bool get_description(uint32_t, pfc::string_base & out) override {
			out = "Likes or unlikes the selected tracks, or the playing track if nothing is selected.";
			return true;
		}
		GUID get_parent() override { return mainmenu_groups::playback; }
		void execute(uint32_t, service_ptr_t<service_base>) override { toggleSelectionOrPlaying(); }
	};
	FB2K_SERVICE_FACTORY(likeCommand);

	// No click API for playlist columns, so a mouse hook on the UI thread matches clicks to the header.
	HHOOK g_hook = nullptr;
	HWND g_pressedList = nullptr;

	bool isPlaylistView(HWND wnd) {
		wchar_t cls[64] = {};
		if (GetClassNameW(wnd, cls, (int)std::size(cls)) == 0) return false;
		if (wcscmp(cls, L"libPPUI:CListControl") != 0) return false;
		return GetAncestor(wnd, GA_ROOT) == core_api::get_main_window();
	}

	bool isLikeColumnHit(HWND list, POINT screenPt) {
		if (!isPlaylistView(list)) return false;
		HWND header = FindWindowExW(list, nullptr, WC_HEADERW, nullptr);
		if (header == nullptr || !IsWindowVisible(header)) return false;

		RECT headerRect;
		GetWindowRect(header, &headerRect);
		if (screenPt.y < headerRect.bottom) return false;

		POINT pt = screenPt;
		ScreenToClient(header, &pt);
		const int count = Header_GetItemCount(header);
		for (int i = 0; i < count; ++i) {
			wchar_t text[64] = {};
			HDITEMW item = {};
			item.mask = HDI_TEXT;
			item.pszText = text;
			item.cchTextMax = (int)std::size(text);
			if (!Header_GetItem(header, i, &item) || wcscmp(text, columnTitle) != 0) continue;
			RECT rc;
			if (Header_GetItemRect(header, i, &rc) && pt.x >= rc.left && pt.x < rc.right) return true;
		}
		return false;
	}

	bool modifierHeld() {
		return GetKeyState(VK_SHIFT) < 0 || GetKeyState(VK_CONTROL) < 0 || GetKeyState(VK_MENU) < 0;
	}

	LRESULT CALLBACK mouseHook(int code, WPARAM msg, LPARAM lp) {
		if (code == HC_ACTION) {
			const auto * info = reinterpret_cast<const MOUSEHOOKSTRUCT *>(lp);
			if (msg == WM_LBUTTONDOWN) {
				g_pressedList = (!modifierHeld() && isLikeColumnHit(info->hwnd, info->pt)) ? info->hwnd : nullptr;
			} else if (msg == WM_LBUTTONUP) {
				const bool release = g_pressedList != nullptr && g_pressedList == info->hwnd && isLikeColumnHit(info->hwnd, info->pt);
				g_pressedList = nullptr;
				if (release) fb2k::inMainThread([] { toggleClickedRow(); });
			}
		}
		return CallNextHookEx(g_hook, code, msg, lp);
	}

	class hookInstaller : public initquit {
	public:
		void on_init() override {
			g_hook = SetWindowsHookExW(WH_MOUSE, mouseHook, nullptr, GetCurrentThreadId());
			if (g_hook == nullptr) console::print("Like Heart: could not install the mouse hook, clicking the column will not work.");
		}
		void on_quit() override {
			if (g_hook != nullptr) UnhookWindowsHookEx(g_hook);
			g_hook = nullptr;
		}
	};
	FB2K_SERVICE_FACTORY(hookInstaller);
}
