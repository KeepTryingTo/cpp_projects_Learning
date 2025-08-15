all: httpd client

LIBS = -lpthread  # 需要放在编译命令的末尾

httpd: httpd.c
	gcc -g -W -Wall -o $@ $< $(LIBS)  # 将 $(LIBS) 移到 $< 之后

client: simpleclient.c
	gcc -W -Wall -o $@ $<

clean:
	rm -f httpd client