CXX = g++
WINDRES = windres
CXXFLAGS = -std=c++20 -O3 -flto -s -mwindows -municode -Wall -Wextra -Iinclude
LDFLAGS = -lwinhttp -lgdiplus -ld2d1 -ldwrite -lgdi32 -luser32 -ldwmapi -lcomctl32 -lcomdlg32 -lole32 -luuid -luxtheme -lshlwapi -lshell32 -static -static-libgcc -static-libstdc++

TARGET = YTLiveChatOverlay.exe
TEST_TARGET = run_tests.exe

SRCS = src/main.cpp \
       src/common/Theme.cpp \
       src/common/ThemeModel.cpp \
       src/common/ThemeManager.cpp \
       src/common/Config.cpp \
       src/net/WinHttpClient.cpp \
       src/net/YouTubeEngine.cpp \
       src/net/EmojiCache.cpp \
       src/ui/ColorEmojiRenderer.cpp \
       src/ui/TextRenderer.cpp \
       src/ui/OverlayWindow.cpp \
       src/ui/SettingsWindow.cpp \
       src/ui/TrayManager.cpp

TEST_SRCS = tests/test_runner.cpp \
            tests/unit/test_json.cpp \
            tests/unit/test_url_parser.cpp \
            tests/unit/test_theme.cpp \
            tests/unit/test_config.cpp \
            tests/unit/test_unicode_multilingual.cpp \
            tests/unit/test_emoji_segmentation.cpp \
            tests/unit/test_theme_system.cpp \
            tests/unit/test_custom_emoji_cache.cpp \
            src/common/Theme.cpp \
            src/common/ThemeModel.cpp \
            src/common/ThemeManager.cpp \
            src/common/Config.cpp \
            src/net/WinHttpClient.cpp \
            src/net/YouTubeEngine.cpp \
            src/net/EmojiCache.cpp \
            src/ui/ColorEmojiRenderer.cpp

RES = src/resource/app.rc
RES_OBJ = src/resource/app.res.o

all: $(TARGET) $(TEST_TARGET)

$(RES_OBJ): $(RES)
	$(WINDRES) -i $(RES) -o $(RES_OBJ) -O coff

$(TARGET): $(SRCS) $(RES_OBJ)
	$(CXX) $(CXXFLAGS) $(SRCS) $(RES_OBJ) -o $(TARGET) $(LDFLAGS)

$(TEST_TARGET): $(TEST_SRCS)
	$(CXX) -std=c++20 -O2 $(TEST_SRCS) -o $(TEST_TARGET) -lwinhttp -lgdiplus -ld2d1 -ldwrite -lcomctl32 -lcomdlg32 -lshlwapi -lole32 -lgdi32 -luser32 -static -static-libgcc -static-libstdc++

clean:
	del /Q $(TARGET) $(TEST_TARGET) $(RES_OBJ) 2>nul || true

test: $(TEST_TARGET)
	./$(TEST_TARGET) --cli

.PHONY: all clean test
