/*
 * Author: lorenzo
 * E-mail: zetatez@icloud.com
 */

/* #define _BSD_SOURCE */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/statvfs.h>

#include <X11/Xlib.h>

char *tzargentina = "GTM-8";

static Display *dpy;

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

void
settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}

char *
mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL)
		return smprintf("");

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		return smprintf("");
	}

	return smprintf("%s", buf);
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char *
loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0)
		return smprintf("");

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

char *
notification(char *base, char *file)
{
    char *path, line[513];
	FILE *fd;

	memset(line, 0, sizeof(line));

	path = smprintf("%s/%s", base, file);
	fd = fopen(path, "r");
	free(path);
	if (fd == NULL)
		return smprintf("");

	if (fgets(line, sizeof(line)-1, fd) == NULL)
		return smprintf("");
	fclose(fd);

    line[strlen(line)-1]='\0';

	return smprintf("%s", line);
}

char *
readfile(char *base, char *file)
{
	char *path, line[513];
	FILE *fd;

	memset(line, 0, sizeof(line));

	path = smprintf("%s/%s", base, file);
	fd = fopen(path, "r");
	free(path);
	if (fd == NULL)
		return NULL;

	if (fgets(line, sizeof(line)-1, fd) == NULL) return NULL;
	fclose(fd);

	return smprintf("%s", line);
}

char *
getbattery(char *base)
{
	char *co, status;
	int descap, remcap;

	descap = -1;
	remcap = -1;

	co = readfile(base, "present");
	if (co == NULL)
		return smprintf("");
	if (co[0] != '1') {
		free(co);
		return smprintf("not present");
	}
	free(co);

	co = readfile(base, "charge_full_design");
	if (co == NULL) {
		co = readfile(base, "energy_full_design");
		if (co == NULL)
			return smprintf("");
	}
	sscanf(co, "%d", &descap);
	free(co);

	co = readfile(base, "charge_now");
	if (co == NULL) {
		co = readfile(base, "energy_now");
		if (co == NULL)
			return smprintf("");
	}
	sscanf(co, "%d", &remcap);
	free(co);

	co = readfile(base, "status");
	if (!strncmp(co, "Discharging", 11)) {
		status = '-';
	} else if(!strncmp(co, "Charging", 8)) {
		status = '+';
	} else {
		status = '^';
	}

	if (remcap < 0 || descap < 0)
		return smprintf("invalid");

	return smprintf("%.0f%%%c", ((float)remcap / (float)descap) * 100, status);
}

char *get_freespace(char *mntpt){
    struct statvfs data;
    double total, used = 0;

    if ( (statvfs(mntpt, &data)) < 0){
		fprintf(stderr, "can't get info on disk.\n");
		return("-");
    } total = (data.f_blocks * data.f_frsize); used  = (data.f_blocks - data.f_bfree) * data.f_frsize ; return(smprintf("%.0f%%", (used/total*100))); } char * gettemperature(char *base, char *sensor)
{
	char *co;

	co = readfile(base, sensor);
	if (co == NULL)
		return smprintf("");
	return smprintf("%02.0f°", atof(co) / 1000);
}

int
parse_netdev(unsigned long long int *receivedabs, unsigned long long int *sentabs)
{
	char buf[255];
	char *datastart;
	static int bufsize;
	int rval;
	FILE *devfd;
	unsigned long long int receivedacc, sentacc;

	bufsize = 255;
	devfd = fopen("/proc/net/dev", "r");
	rval = 1;

	// ignore the first two lines of the file
	fgets(buf, bufsize, devfd);
	fgets(buf, bufsize, devfd);

	while (fgets(buf, bufsize, devfd)) {
	    if ((datastart = strstr(buf, "lo:")) == NULL) {
		datastart = strstr(buf, ":");

		// with thanks to the conky project at http://conky.sourceforge.net/
		sscanf(datastart + 1, "%llu  %*d     %*d  %*d  %*d  %*d   %*d        %*d       %llu", &receivedacc, &sentacc);
		*receivedabs += receivedacc;
		*sentabs += sentacc;
		rval = 0;
	    }
	}

	fclose(devfd);
	return rval;
}

void
calculate_speed(char *speedstr, unsigned long long int newval, unsigned long long int oldval)
{
	double speed;
	speed = (newval - oldval) / 1024.0;
	if (speed > 1024.0) {
	    speed /= 1024.0;
	    sprintf(speedstr, "%.3f mb/s", speed);
	} else {
	    sprintf(speedstr, "%.2f kb/s", speed);
	}
}

char *
get_netusage(unsigned long long int *rec, unsigned long long int *sent)
{
	unsigned long long int newrec, newsent;
	newrec = newsent = 0;
	char downspeedstr[15], upspeedstr[15];
	static char retstr[42];
	int retval;

	retval = parse_netdev(&newrec, &newsent);
	if (retval) {
	    fprintf(stdout, "Error when parsing /proc/net/dev file.\n");
	    exit(1);
	}

	calculate_speed(downspeedstr, newrec, *rec);
	calculate_speed(upspeedstr, newsent, *sent);

	/* sprintf(retstr, "down: %s up: %s", downspeedstr, upspeedstr); */
	sprintf(retstr, "⇊-%s ⇈-%s", downspeedstr, upspeedstr);

	*rec = newrec;
	*sent = newsent;
	return retstr;
}


/*
int
main(void)
{
	char *status;
	char *tm;
	char *bat;
	char *msg;
	char *rootfs;
	char *avgs;
 	char *netstats;
	static unsigned long long int rec, sent;
	char *t0, *t1, *t2, *t3, *t4, *t5, *t6, *t7, *t8;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(60)) {
		tm = mktimes("%a %b/%d %Y %Z ∫_%H:%M e^r(t)du ", tzargentina);
		bat = getbattery("/sys/class/power_supply/BAT0");
        msg = notification("/home/lorenzo", ".notification.msg");
		rootfs = get_freespace("/");
		avgs = loadavg();
		netstats = get_netusage(&rec, &sent);
		t0 = gettemperature("/sys/class/thermal/thermal_zone0", "temp");
		t1 = gettemperature("/sys/class/thermal/thermal_zone1", "temp");
		t2 = gettemperature("/sys/class/thermal/thermal_zone2", "temp");
		t3 = gettemperature("/sys/class/thermal/thermal_zone3", "temp");
		t4 = gettemperature("/sys/class/thermal/thermal_zone4", "temp");
		t5 = gettemperature("/sys/class/thermal/thermal_zone5", "temp");
		t6 = gettemperature("/sys/class/thermal/thermal_zone6", "temp");
		t7 = gettemperature("/sys/class/thermal/thermal_zone7", "temp");
		t8 = gettemperature("/sys/class/thermal/thermal_zone8", "temp");

		status = smprintf("Arch %s %s %s %s%s%s%s%s%s%s%s%s %s %s %s", bat, rootfs, netstats, t0, t1, t2, t3, t4, t5, t6, t7, t8, avgs, tm, msg);
		setstatus(status);

        free(t0); free(t1); free(t2); free(t3); free(t4); free(t5); free(t6); free(t7); free(t8);
		free(tm);
		free(bat);
		free(msg);
		free(rootfs);
		free(avgs);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}
*/

int
main(void)
{
	char *status;
	char *tm;
	char *bat;
	char *msg;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(1)) {
		tm = mktimes("%a %b/%d,%Y∫_%H:%M:%S e^r(t)du", tzargentina);
		bat = getbattery("/sys/class/power_supply/BAT0");
        msg = notification("/home/lorenzo", ".notification.msg");
        status = smprintf(" %s [%s] [%s] ", tm, msg, bat);
		setstatus(status);

		free(tm);
		free(bat);
		free(msg);
		free(status);
	}

	XCloseDisplay(dpy);

	return 0;
}

