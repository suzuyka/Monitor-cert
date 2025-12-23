CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -fPIC \
    -I/usr/include/x86_64-linux-gnu/qt6 \
    -I/usr/include/x86_64-linux-gnu/qt6/QtWidgets \
    -I/usr/include/x86_64-linux-gnu/qt6/QtCore \
    -I/usr/include/x86_64-linux-gnu/qt6/QtGui
LIBS = -lsqlite3 -lQt6Widgets -lQt6Core -lQt6Gui

SRCS = main.cpp database.cpp mainwindow.cpp loginwindow.cpp registerwindow.cpp \
       dataformwidget.cpp profilewidget.cpp sql_repository.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = monitor

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

