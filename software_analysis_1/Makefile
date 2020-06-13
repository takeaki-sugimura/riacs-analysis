CXXFLAGS = -O2 -Wall
SRCDIR = ./src
DSTDIR = ./bin
OBJS = $(DSTDIR)/srs_file.o $(DSTDIR)/srs_proc.o $(DSTDIR)/serendipiter5_img_3t3l1.o $(DSTDIR)/serendipiter5_img_chlam.o $(DSTDIR)/serendipiter5_img_euglena.o 

$(DSTDIR)/srs_proc: $(OBJS)
	g++ -o $@ $^ `pkg-config --libs opencv`

$(DSTDIR)/%.o: $(SRCDIR)/%.cpp
	g++ $(CXXFLAGS) -o $@ -std=c++11 -c $< `pkg-config --cflags opencv`

clean:
	rm -f $(DSTDIR)/srs_proc $(DSTDIR)/*.o
