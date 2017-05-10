#include <stdio.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

int main() {
    int i;
    time_t rawtime;
    struct tm * timeinfo;
    char buf[80];
    memset(buf, '\0', 80);

    time (&rawtime);
    timeinfo = localtime (&rawtime);

    strftime (buf, 80, "%l:%M%p, %A, %B %e, %G", timeinfo);

    i = 0;
    if (buf[0] == ' ') {
	while (buf[i]) {
	    buf[i] = buf[i+1];
	    i++;
	}
    }
    buf[i] = '\0';

    i = 0;
    while (buf[i]) {
	if (i < 8 && (buf[i] == 'A' || buf[i] == 'P' || buf[i] == 'M'))
	    buf[i] = tolower(buf[i]);
	i++;
    }

    FILE *fout = fopen("currentTime.txt", "w");
    fprintf(fout, "%s\n\n", buf);
    return 0;
}

