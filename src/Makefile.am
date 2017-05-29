INCLUDES= -I./mgi/include -I./mgi -I./mgi/softkeyboard/softkeyboard -I./mgi/softkeyboard/animate
NULL=

noinst_PROGRAMS = mdhome

HOMESRC= mgi/hzinput.c

SOFTKEYBOARD= \
	mgi/softkeyboard/softkeywindow.c \
	mgi/softkeyboard/tooltip.h \
	mgi/softkeyboard/tooltip.c \
	mgi/softkeyboard/animate/animate.h \
	mgi/softkeyboard/animate/animate_sence.c \
	mgi/softkeyboard/animate/instruction.h \
	mgi/softkeyboard/animate/my_debug.h \
	mgi/softkeyboard/animate/p-code.c \
	mgi/softkeyboard/animate/p-code.h \
	mgi/softkeyboard/animate/timeline.c \
	mgi/softkeyboard/animate/common_animates/alpha_animate.c \
	mgi/softkeyboard/animate/common_animates/common_animates.c \
	mgi/softkeyboard/animate/common_animates/common_animates.h \
	mgi/softkeyboard/softkeyboard/common.c \
	mgi/softkeyboard/softkeyboard/common.h \
	mgi/softkeyboard/softkeyboard/en_kbd.c \
	mgi/softkeyboard/softkeyboard/ime.c \
	mgi/softkeyboard/softkeyboard/ime.h \
	mgi/softkeyboard/softkeyboard/num_kbd.c \
	mgi/softkeyboard/softkeyboard/pinyin_kbd.c \
	mgi/softkeyboard/softkeyboard/punct_kbd.c \
	mgi/softkeyboard/softkeyboard/resource.c \
	mgi/softkeyboard/softkeyboard/softkeyboard.h \
	$(NULL)

MGIHOME_SRC= \
	mgi/contbmp.c \
    mgi/jt_engine.c \
    mgi/mgpti_dict.c \
	mgi/jt_input.h \
    mgi/mgpti_dict.h \
    mgi/mgpti_user_dict.c \
	mgi/hw_engine.c \
    mgi/keyboard.c \
    mgi/mgpti_dict_ops.c \
    mgi/mgpti_user_dict.h \
	mgi/hwinput.c \
    mgi/mgime.c \
    mgi/mgpti_dict_ops.h \
	mgi/ime_pinyin.c \
    mgi/mgphone.c \
    mgi/wt_engine.c \
	mgi/mgpti_helper.c \
    mgi/wt_input.h \
	mgi/ime_tab_pinyin.h \
    mgi/mgpti.c \
    mgi/mgpti_helper.h \
	mgi/mgime_tab.h \
    mgi/hw.h \
    mgi/mgi_helper.h \
    $(NULL)

imefiles= \
    $(MGIHOME_SRC) \
	$(SOFTKEYBOARD) \
    $(HOMESRC) \
    $(NULL)

if MINIGUI_V3
MAGNIFIER = magnifier3.cpp magnifier3.h
else
MAGNIFIER = magnifier.cpp magnifier.h
endif

mdhome_SOURCES = \
			mdhome.cpp \
			mdhome.h \
			toolbar.cpp \
			toolbar.h \
			callback.cpp \
			tabpage.h \
			tabpage.cpp \
			network.cpp \
			network.h \
			panpages.cpp \
			panpages.h \
			startup.cpp \
			error.cpp \
			scrollpanel.cpp \
			scrollpanel.h \
            savefile.cpp \
            progress_dialog.cpp \
            clientcert.cpp \
            verifycert.cpp \
			prompt_dialog.cpp \
			prompt_dialog.h \
            tooltipwin.cpp \
            tooltipwin.h \
			mdolphin_auth.cpp \
			mdolphin_auth.h \
			$(MAGNIFIER) \
            $(imefiles) \
			$(NULL)

if MINIGUI_V3
MGLIBS= -lmgutils 
else
MGLIBS= -lmgext
endif

MDLDFLAGS = 
mdhome_LDFLAGS = $(MGLIBS) $(LIBS) $(MDLDFLAGS)

EXTRA_DIST = \
			res-800/magnifier.png \
			res-800/normal.png \
			res-800/progress_bar.png \
			res-800/tab.png \
			res-800/toolbar_bk.png \
			res-800/toolbar.png \
			res/err_bk.png \
			res/err_button.png \
			res/err_mark.png \
 			res/hbar.png \
			res/hbar2.png \
			res/htrack.png \
			res/htrack2.png \
			res/magnifier.png \
			res/progress_bar.png \
			res/startup.png \
			res/tab.png \
			res/toolbar.png \
			res/normal.png \
			res/toolbar_bk.png \
			res/index.html \
			res/en_index.html \
			res/index_files/cell.png \
			res/tabview.html \
			res/tabview/chinese_sites_on.png \
			res/tabview/chinese_sites.png \
			res/tabview/english_sites_on.png \
			res/tabview/english_sites.png \
			res/tabview/http_on.png \
			res/tabview/http.png \
			res/tabview/network_setting_on.png \
			res/tabview/network_setting.png \
			res/network.html \
			res/network1.html \
			res/network/cancel_down.png \
			res/network/cancel_up.png \
			res/network/dhcp_down.png \
			res/network/dhcp_icon.png \
			res/network/dhcp_up.png \
			res/network/form_label.png \
			res/network/save_down.png \
			res/network/save_up.png \
			res/network/setbutton_down.png \
			res/network/setbutton_up.png \
			res/network/static_down.png \
			res/network/static_icon.png \
			res/network/static_up.png \
			res/network/status_bg.png \
			res/input.html \
			res/input/input.png \
			res/input/go.png \
			res/input/go_on.png \
			res/input/01.png \
			res/input/02.png \
			res/input/03.png \
			res/input/04.png \
			res/input/05.png \
			res/input/06.png \
			res/input/07.png \
			res/input/08.png \
			res/input/09.png \
			res/input/10.png \
			res/input/11.png \
			res/input/12.png \
			res/input/13.png \
			res/input/14.png \
			res/input/15.png \
			res/input/16.png \
			res/input/17.png \
			res/input/18.png \
			res/input/19.png \
			res/input/20.png \
			res/input/21.png \
			res/input/22.png \
			res/input/23.png \
			res/input/24.png \
			res/input/25.png \
			res/input/26.png \
			res/input/27.png \
			res/input/28.png \
			res/input/29.png \
			res/input/30.png \
			res/input/31.png \
			res/input/32.png \
			res/input/33.png \
			res/input/34.png \
			res/input/35.png \
			res/input/36.png \
			res/input/37.png \
			res/input/38.png \
			res/input/39.png \
			res/input/40.png \
			res/input/41.png \
			res/input/42.png \
			res/input/43.png \
			res/input/44.png \
			res/input/45.png \
			res/input/46.png \
			res/input/47.png \
			res/input/48.png \
			res/input/49.png \
			res/input/50.png \
			res/input/01_on.png \
			res/input/02_on.png \
			res/input/03_on.png \
			res/input/04_on.png \
			res/input/05_on.png \
			res/input/06_on.png \
			res/input/07_on.png \
			res/input/08_on.png \
			res/input/09_on.png \
			res/input/10_on.png \
			res/input/11_on.png \
			res/input/12_on.png \
			res/input/13_on.png \
			res/input/14_on.png \
			res/input/15_on.png \
			res/input/16_on.png \
			res/input/17_on.png \
			res/input/18_on.png \
			res/input/19_on.png \
			res/input/20_on.png \
			res/input/21_on.png \
			res/input/22_on.png \
			res/input/23_on.png \
			res/input/24_on.png \
			res/input/25_on.png \
			res/input/26_on.png \
			res/input/27_on.png \
			res/input/28_on.png \
			res/input/29_on.png \
			res/input/30_on.png \
			res/input/31_on.png \
			res/input/32_on.png \
			res/input/33_on.png \
			res/input/34_on.png \
			res/input/35_on.png \
			res/input/36_on.png \
			res/input/37_on.png \
			res/input/38_on.png \
			res/input/39_on.png \
			res/input/40_on.png \
			res/input/41_on.png \
			res/input/42_on.png \
			res/input/43_on.png \
			res/input/44_on.png \
			res/input/45_on.png \
			res/input/46_on.png \
			res/input/47_on.png \
			res/input/48_on.png \
			res/input/49_on.png \
			res/input/50_on.png 

SUBDIRS= mgi 

DIST_SUBDIRS= mgi 
