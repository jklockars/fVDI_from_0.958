#undef USE_FLOAT
#undef NO_COUNTER
#undef NO_INTERRUPT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __PUREC__
   #define USE_FLOAT
   #include <tos.h>
#else
   #include <osbind.h>
#endif

#define Abs(x) (((x) >= 0) ? x : -x)

#define VERSION "v0.72"
#define SUPPORTED_TABLE 0x10
#define MAX_COUNT 192

struct Profile {
	long count;
	long time_interrupt;
	long time_counter;
	long dummy;
} ;

struct Info {
	unsigned char version;
	unsigned char table_version;
	int flags;
	long start_time;
	struct Profile *prof;
	int table_size;
	char fives;
	char elevenths;
	void (*reset)(void);
	long (*unlink)(void);
	long (*relink)(void);
} ;
	
struct Profile *prof;
struct Info *info;

long get_cookie(char *cname)
{
   long oldstack, *ptr, value, name;

   name = 0;
   while(*cname)
      name = (name << 8) | (int)*cname++;

   oldstack = (long)Super(0L);
   ptr = (long *)*(long *)0x5a0;

   if (ptr != NULL) {
      while ((*ptr != 0) && (*ptr != name))
         ptr += 2;
      if (*ptr == name)      /* The if statement is new */
         value = ptr[1];
      else
         value = -1;
   } else
      value = -1;             /*  This used to be 0 */

   Super((void *)oldstack);
   return value;
}

long get_time(void)
{
   long oldstack, value;

   oldstack = (long)Super(0L);
   value = *(long *)0x4ba;
   Super((void *)oldstack);
   return value;
}

long clicks(struct Profile *prof)
{
	long time_clicks, time_interrupt, time_counter;
	
	time_interrupt = prof->time_interrupt;
	time_counter = prof->time_counter;
#ifdef NO_COUNTER
	time_clicks = time_interrupt * MAX_COUNT;
#else
 #ifdef NO_INTERRUPT
	time_clicks = time_counter;
 #else
	time_clicks = time_interrupt * MAX_COUNT + time_counter;
 #endif
#endif
	return time_clicks;  
}

long clicks_to_1000(long time_clicks)
{
	long time_1000;
	
	if (Abs(time_clicks) < 400000000L)		/* 2e9 / 5 */
		time_1000 = (time_clicks * 5 + MAX_COUNT / 2) / MAX_COUNT;
	else
		time_1000 = 5 * (time_clicks / MAX_COUNT);	
	return time_1000;
}

void output(FILE *outfile, int i, long this, long so_far, long total)
{
	int ex_5, ex_11;
	int n, m, length;
	long time_clicks, time_1000;
#ifdef USE_FLOAT
	float percentage_this, percentage_so_far;
#else
	long percentage_this, percentage_so_far;
#endif
	char buf[100];
	
	time_clicks = clicks(&prof[i]);
	time_1000 = clicks_to_1000(time_clicks);

	ex_5 = info->fives - 1;
	ex_11 = info->elevenths - 1;
	
	m = 0;
	if (i == info->table_size)
		m = -1;
	else if (i > 11 + ex_5 + ex_11)
		n = i - ex_5 - ex_11;
	else if (i >= 11 + ex_5) {
		m = 11;
		n = i - 11 - ex_5;
	} else if (i > 5 + ex_5)
		n = i - ex_5;
	else if (i >= 5) {
		m = 5;
		n = i - 5;
	} else
		n = i;

	if (m == 0)
		length = sprintf(buf, "Function: %3d     Called: %9ld   Time: %4ld.%03ld (%ld)",
		                 n, prof[i].count, time_1000 / 1000, time_1000 % 1000, time_clicks);
	else if ((m == 5) || (m == 11))
		length = sprintf(buf, "Function: %2d-%-2d   Called: %9ld   Time: %4ld.%03ld (%ld)",
		                 m, n, prof[i].count, time_1000 / 1000, time_1000 % 1000, time_clicks);
	else
		length = sprintf(buf, "Unknown functions   Called: %9ld   Time: %4ld.%03ld (%ld)",
		                 prof[i].count, time_1000 / 1000, time_1000 % 1000, time_clicks);
	
	if (total) {
		memset(&buf[length], ' ', 66 - length);
#ifdef USE_FLOAT
		percentage_this = (float)this * 100 / total;
		percentage_so_far = (float)so_far * 100 / total;
		sprintf(&buf[66], "(%3.2f/%.2f)\n", percentage_this, percentage_so_far);
#else
		percentage_this = (long long)this * 10000 / total;
		percentage_so_far = (long long)so_far * 10000 / total;
		sprintf(&buf[66], "(%3ld.%02ld/%3ld.%02ld)\n",
		        percentage_this / 100, percentage_this % 100,
		        percentage_so_far / 100, percentage_so_far % 100);
#endif
	} else
		strcat(buf, "\n");
	
	fprintf(outfile, buf);
}

int cmp_count(const int *elem1, const int *elem2)
{
	if (prof[*elem1].count < prof[*elem2].count)
		return 1;
	else if (prof[*elem1].count > prof[*elem2].count)
		return -1;
	else
		return 0;
}

int cmp_time(const int *elem1, const int *elem2)
{
	long time1, time2;
	
	time1 = clicks(&prof[*elem1]);
	time2 = clicks(&prof[*elem2]);

	if (time1 < time2)
		return 1;
	else if (time1 > time2)
		return -1;
	else
		return 0;
}

void main(void)
{
	int i, n, nonzero;
	long tmp;
	FILE *outfile;
	int *count_order, *time_order;
	long total_time, vdi_clicks, vdi_1000;
	long vdi_count, this, accumulated;
	
	if ((tmp = get_cookie("VDIp")) == -1)
		exit(-1);
	
	info = (struct Info *)tmp;

	if (info->table_version != SUPPORTED_TABLE)
		exit(-1);

	prof = info->prof;

	if ((outfile = fopen("vdi_prof.txt", "w")) == NULL)
		exit(-1);

	total_time = (get_time() - info->start_time) / 2;

	nonzero = 0;
	vdi_clicks = 0;
	vdi_count = 0;
	for(i = 0; i <= info->table_size; i++)
		if (prof[i].count) {
			vdi_clicks += clicks(&prof[i]);
			vdi_count += prof[i].count;
			nonzero++;
		}
	vdi_1000 = clicks_to_1000(vdi_clicks);

	count_order = (int *)Malloc(2 * nonzero * sizeof(int));
	if (!count_order) {
		fclose(outfile);
		exit(-1);
	}
	time_order = &count_order[nonzero];

	
	fprintf(outfile, "VDI log analyzer program, %s.\n", VERSION);
	fprintf(outfile, "Working on data from logger v%d.%d.\n",
	        info->version >> 4, info->version & 0x0f);
	fprintf(outfile, "Data collected during %ld.%02ld seconds of which\n",
	        total_time / 100, total_time % 100);
	fprintf(outfile, "%ld calls spent %ld.%02ld seconds (%ld) in the VDI.\n",
	        vdi_count, vdi_1000 / 1000, (vdi_1000 % 1000) / 10, vdi_clicks);
	fprintf(outfile, "\n");

	fprintf(outfile, "Sorted by function number\n");

	n = 0;
	for(i = 0; i <= info->table_size; i++)
		if (prof[i].count) {
			output(outfile, i, 0, 0, 0);
			count_order[n] = time_order[n] = i;
			n++;
		}

	qsort(count_order, nonzero, sizeof(int), cmp_count);
	qsort(time_order, nonzero, sizeof(int), cmp_time);

	fprintf(outfile, "\n\nSorted by number of calls\n");
	
	accumulated = 0;
	for(i = 0; i < nonzero; i++) {
		this = prof[count_order[i]].count;
		accumulated += this;
		output(outfile, count_order[i], this, accumulated, vdi_count);
	}

	fprintf(outfile, "\n\nSorted by total time taken\n");
	
	accumulated = 0;
	for(i = 0; i < nonzero; i++) {
		this = clicks(&prof[time_order[i]]);
		accumulated += this;
		output(outfile, time_order[i], this, accumulated, vdi_clicks);
	}

	Mfree(count_order);
		
	fclose(outfile);
}
