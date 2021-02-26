#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LINE_LEN 132
#define MAX_EVENTS 500
#define MAX_CPL 81

/* Global Variables */
int cur;

struct event {
    char dts[MAX_CPL];
    char dte[MAX_CPL];
    char freq[MAX_CPL];
    char wkst[MAX_CPL];
    char until[MAX_CPL];
    char byday[MAX_CPL];
    char loc[MAX_CPL];
    char sum[MAX_CPL];
};

struct event event_array[MAX_EVENTS];

/*
 * function dt_format.
 *
 * given a date-time, creates a more readable version of the
 * calendar date by using some c-library routines. for example,
 * if the string in "dt_time" corresponds to:
 *
 *   20190520t111500
 *
 * then the string stored at "formatted_time" is:
 *
 *   may 20, 2019 (mon).
 *
 *   from timeplay.c FILE provided for this assignment
 *
 */
void dt_format(char *formatted_time, const char *dt_time, const int len)
{
    struct tm temp_time;
    time_t    full_time;
    char      temp[5];

    /*
     * Ignore for now everything other than the year, month and date.
     * For conversion to work, months must be numbered from 0, and the
     * year from 1900.
     */
    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(dt_time, "%4d%2d%2d",
        &temp_time.tm_year, &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    full_time = mktime(&temp_time);
    strftime(formatted_time, len, "%B %d, %Y (%a)",
        localtime(&full_time));
}

/*
 * function dt_increment:
 *
 * given a date-time, it adds the number of days in a way that
 * results in the correct year, month, and day. for example,
 * if the string in "before" corresponds to:
 *
 *   20190520T111500
 *
 * then the datetime string stored in "after", assuming that
 * "num_days" is 100, will be:
 *
 *   20190828T111500
 *
 * which is 100 days after may 20, 2019 (i.e., august 28, 2019).
 *
 *   from timeplay.c FILE provided for this assignment
 */
void dt_increment(char *after, const char *before, int const num_days)
{
    struct tm temp_time, *p_temp_time;
    time_t    full_time;
    char      temp[5];

    memset(&temp_time, 0, sizeof(struct tm));
    sscanf(before, "%4d%2d%2d", &temp_time.tm_year,
        &temp_time.tm_mon, &temp_time.tm_mday);
    temp_time.tm_year -= 1900;
    temp_time.tm_mon -= 1;
    temp_time.tm_mday += num_days;

    full_time = mktime(&temp_time);
    after[0] = '\0';
    strftime(after, 9, "%Y%m%d", localtime(&full_time));
    strncpy(after + 8, before + 8, MAX_LINE_LEN - 8);
    after[MAX_LINE_LEN - 1] = '\0';
}


/*
 * am_pm
 * take a character array which consists of only the time value
 * so if DTSTART is
 *
 *      20210214T115000
 *
 *  then the parameter passed to this function is
 *
 *      11500
 *
 *  which will then be calculated to determine whether that time
 *  is in the morning or afternoon
 *
 *  Function will return "AM" for morning and "PM" for afternoon
 *
 * */
char *am_pm(char *time) {
    struct tm temp_time;
    char *pm = "PM";
    char *am = "AM";

    sscanf(time, "%2d", &temp_time.tm_hour);
    if (temp_time.tm_hour == 12) {
        return pm;
    } else if (temp_time.tm_hour >= 13) {
        return pm;
    } else {
        return am;
    }
}


/*
 * sorts two elements in an array of the events
 * only called by qsort
 *
 * function is not passed any written parameter, but is handled
 * by the function qsort itself
 *
 */

int sort_elements(const void *e1, const void *e2) {
    struct event *ie1 = (struct event*)e1;
    struct event *ie2 = (struct event*)e2;

    if (strcmp(ie1->dts,ie2->dts) < 0) {
        return -1;
    } else if (strcmp(ie1->dts,ie2->dts) == 0) {
        return 0;
    } else {
        return 1;
    }
}


/* print_events
 *
 * takes the events within a certain range of dates and prints those events
 *
 * function is also passed the total number of events in the file
 * as well as the array of events. It will then format and print the
 * array of events according to the format required
 *
 * */
void print_events(int from_yy, int from_mm, int from_dd,
    int to_yy, int to_mm, int to_dd, int total_events, struct event *pevent_array){
    int cur_dash;
    int i = 0;
    int first_el = 0;
    struct tm temp_dt_s, temp_dt_e;
    char *date_s, *time_s, *date_e, *time_e, prev_date[9], copy_dts[MAX_LINE_LEN], copy_dte[MAX_LINE_LEN];
    char output[MAX_LINE_LEN];

    while(i < total_events) {
        strncpy(copy_dts,pevent_array[i].dts,MAX_LINE_LEN);
        strncpy(copy_dte,pevent_array[i].dte,MAX_LINE_LEN);

        date_s = strtok(copy_dts, "T");
        time_s = strtok(NULL, "\n");
        date_e = strtok(copy_dte, "T");
        time_e = strtok(NULL, "\n");

        sscanf(date_s, "%4d%2d%2d", &temp_dt_s.tm_year, &temp_dt_s.tm_mon, &temp_dt_s.tm_mday);
        sscanf(date_e, "%4d%2d%2d", &temp_dt_e.tm_year, &temp_dt_e.tm_mon, &temp_dt_e.tm_mday);
        /* Check if dates are within range */
        if (temp_dt_s.tm_year >= from_yy && temp_dt_e.tm_year <= to_yy && ((temp_dt_s.tm_mon >= from_mm && temp_dt_e.tm_    mon < to_mm && temp_dt_s.tm_mday >= from_dd) || (temp_dt_s.tm_mon > from_mm && temp_dt_e.tm_mon <= to_mm && temp_dt_e.tm    _mday <= to_dd) || (temp_dt_s.tm_mon > from_mm && temp_dt_e.tm_mon < to_mm) ||  (temp_dt_s.tm_mon == from_mm && temp_dt_    e.tm_mon == to_mm && (temp_dt_s.tm_mday >= from_dd && temp_dt_s.tm_mday <= to_dd)))) {

            sscanf(time_s, "%2d%2d", &temp_dt_s.tm_hour, &temp_dt_s.tm_min);
            sscanf(time_e, "%2d%2d", &temp_dt_e.tm_hour, &temp_dt_e.tm_min);

            if (i == 0 || strncmp(pevent_array[i].dts,pevent_array[i-1].dts,9)) {
                if (first_el > 0) {
                    printf("\n");
                }

                /* printing date and dashes */
                strncpy(copy_dts,pevent_array[i].dts,MAX_LINE_LEN);
                strncpy(copy_dte,pevent_array[i].dte,MAX_LINE_LEN);
                dt_format(output, copy_dts, MAX_LINE_LEN);
                printf("%s\n", output);
                cur_dash = 0;
                while(cur_dash < strlen(output)){
                    printf("-");
                    cur_dash++;
                }
                printf("\n");
            }

            /* printing hour:minute for start time */
            if (temp_dt_s.tm_hour > 12) {
                temp_dt_s.tm_hour -= 12;
                printf("%2d:%02d ", temp_dt_s.tm_hour, temp_dt_s.tm_min);
            } else {
                printf("%2d:%02d ", temp_dt_s.tm_hour, temp_dt_s.tm_min);
            }

            /* printing am/pm for start time */
            printf("%s to ", am_pm(time_s));

            /* printing hour:minute for end time */
            if (temp_dt_e.tm_hour > 12) {
                temp_dt_e.tm_hour -= 12;
                printf("%2d:%02d ", temp_dt_e.tm_hour, temp_dt_e.tm_min);
            } else {
                printf("%2d:%02d ", temp_dt_e.tm_hour, temp_dt_e.tm_min);
            }

            /* printing am/pm for end time */
            printf("%s: ", am_pm(time_e));

            /* printing summary */
            if (event_array[i].sum != NULL) {
                printf("%s ", event_array[i].sum);
            }

            /* printing location */
            printf("{{%s}}\n", event_array[i].loc);
            first_el++;

            }
        i++;
    }
}




int main(int argc, char *argv[])
{
    int from_y = 0, from_m = 0, from_d = 0;
    int to_y = 0, to_m = 0, to_d = 0;
    char *filename = NULL;
    int i;

    for (i = 0; i < argc; i++) {
        if (strncmp(argv[i], "--start=", 8) == 0) {
            sscanf(argv[i], "--start=%d/%d/%d", &from_y, &from_m, &from_d);
        } else if (strncmp(argv[i], "--end=", 6) == 0) {
            sscanf(argv[i], "--end=%d/%d/%d", &to_y, &to_m, &to_d);
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            filename = argv[i]+7;
        }
    }

    if (from_y == 0 || to_y == 0 || filename == NULL) {
        fprintf(stderr,
            "usage: %s --start=yyyy/mm/dd --end=yyyy/mm/dd --file=icsfile\n",
            argv[0]);
        exit(1);
    }
    {
            /* declaring variables */
            char *date_start,*date_end, *time_start, *time_end;
            char *category, line[81], *info, *dates, *datee, *timee, *times;
            int updated, new_days;
            char output_dts[MAX_LINE_LEN],output_dte[MAX_LINE_LEN];

            cur = updated = 0;
            new_days = 7;

            /* parsing the file */
            FILE *fp_stream;
            fp_stream = fopen(filename, "r");

            while(fgets(line,81,fp_stream) != NULL) {
                category = strtok(line, ":");

                /* finding out what category you're working with & append to specific lists */
                 if (!strcmp(category, "DTSTART")) {
                    info = strtok(NULL,"\n");
                    strncpy(event_array[cur].dts,info,strlen(info));

                } else if (!strcmp(category, "DTEND")) {
                    info = strtok(NULL,"\n");
                    strncpy(event_array[cur].dte,info,strlen(info));

                } else if (!strcmp(category, "RRULE")) {
                    info = strtok(NULL,"=");
                    if (!strcmp(info,"FREQ")) {
                        info = strtok(NULL,";");
                        strncpy(event_array[cur].freq,info,strlen(info));
                        info = strtok(NULL,"=");
                    }
                    if (!strcmp(info,"WKST")) {
                        info = strtok(NULL,";");
                        strncpy(event_array[cur].wkst,info,strlen(info));
                        info = strtok(NULL,"=");
                    }
                    if (!strcmp(info,"UNTIL")) {
                        info = strtok(NULL,";");
                        strncpy(event_array[cur].until,info,strlen(info));
                        info = strtok(NULL,"=");
                    }
                    if (!strcmp(info,"BYDAY")) {
                        info = strtok(NULL,";");
                        strncpy(event_array[cur].byday,info,strlen(info));
                        info = strtok(NULL,"=");
                    }

                } else if (!strcmp(category, "LOCATION")) {
                    info = strtok(NULL,"\n");
                    if (info != NULL) {
                        strncpy(event_array[cur].loc,info,strlen(info));
                    }

                } else if (!strcmp(category, "SUMMARY")) {
                    info = strtok(NULL,"\n");
                    strncpy(event_array[cur].sum,info,strlen(info));

                    if (!strcmp(event_array[cur].freq, "WEEKLY")) {
                        updated = cur;
                        new_days = 7;
                        while (strcmp(event_array[updated].dts, event_array[cur].until)<0) {
                                dt_increment(output_dts, event_array[updated].dts, new_days);
                                dt_increment(output_dte, event_array[updated].dte, new_days);
                                strncpy(event_array[updated+1].dts, output_dts, strlen(output_dts));
                                strncpy(event_array[updated+1].dte, output_dte, strlen(output_dte));
                                strncpy(event_array[updated].loc, event_array[cur].loc, strlen(event_array[cur].loc));
                                strncpy(event_array[updated].sum, event_array[cur].sum, strlen(event_array[cur].sum));
                                updated++;
                        }
                    }
                    if (updated > cur) {
                        cur = updated-1;
                    }
                    cur += 1;
                }
            }
            fclose(fp_stream);
            qsort(event_array,cur,sizeof(struct event),sort_elements);

            /* Starting calling your own code from this point. */
            print_events(from_y,from_m,from_d,to_y,to_m,to_d,cur,&event_array[0]);
    }
    /* End program */
    exit(0);
}
