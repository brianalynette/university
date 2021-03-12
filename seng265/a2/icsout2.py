#!/usr/bin/env python3

import sys
import argparse
import datetime


"""
    dict_events() takes an iCal file as a parameter
    and puts all information into a dictionary with the
    start time for keys and lists of info for values
"""
def dict_events(f):
    dofe = {} # dictionary of events
    loi = [] # list of info
    loir = [] # list of info for repeated events
    freq = None
    until = None

    for line in f:
        category,info = line.split(":")

        if (category == "DTSTART"):
            cur_dts = create_dt(info)

        elif (category == "DTEND"):
            cur_dte = create_dt(info) 
            loi.append(cur_dte)
            
        elif (category == "RRULE"):
            rrule = info.split(";")
            freqstr = rrule[0]
            
            if (len(rrule) == 4):
                wkst = rrule[1]
                untilstr = rrule[2]
                bydaystr = rrule[3]
            
            elif (len(rrule) == 3):
                untilstr = rrule[1]
                bydaystr = rrule[2]

            else:
                untilstr = rrule[1]

            freq = freqstr.split("=")
            until = untilstr.split("=")
            cur_until = create_dt(until[1]) 

        elif (category == "LOCATION"):
            if (info != None):
                loi.append(info)

        elif (category == "SUMMARY"):
            loi.append(info)

            if (freq != None):
                loi.append(freq[1])
                loi.append(cur_until)
                dofe = add_repeats(dofe,loi,cur_dts,cur_dte,cur_until)
               
            dofe[cur_dts] = loi
            freq = None
            until = None
            loi = []

    return dofe


""" 
    create_dt() takes a string of the format
        20210214T180000
    and returns a datetime object
"""
def create_dt(dt_string):
        date,time = dt_string.split("T")
        return datetime.datetime(int(date[:4]),int(date[4:6]),int(date[6:]),
                                int(time[:2]),int(time[2:4]),int(time[4:]))


"""
    add_repeats() takes the start date and until date as parameters
    and adds each repeating event to the dictionary of events
"""
def add_repeats(dofe_upd,loi,dstart,dend,cur_until):
    new_days = 7
    dts_rep = dstart
    dte_rep = dend
    delta = datetime.timedelta(7)

    while (dts_rep+delta <= cur_until):
        dts_rep += delta
        dte_rep += delta
        loir = [dte_rep]
        loir.append(loi[1]) # append location
        loir.append(loi[2]) # append summary
        loir.append(loi[3]) # append freq
        loir.append(loi[4]) # append until
        dofe_upd[dts_rep] = loir
    
    return dofe_upd


""" 
    print_events() will take the restricting dates as parameters
    as well as the list of events and will print the date and dashes,
    then it will call print_tsl() to print the time, summary, and location
    of said events
"""
def print_events(dictevs,start,end):
    first_el = 0
    prev_date = datetime.date(1,1,1)

    for i in sorted(dictevs.keys()):
        if (i.date() >= start and i.date() <= end):
            
            if (first_el > 0 and (i.date() != prev_date)):
                print()
            
            if (i.date() != prev_date):
                ds_output = i.strftime("%B %d, %Y (%a)")
                print(ds_output)
                print("-" * len(ds_output))

            te_output = dictevs[i][0].strftime("%_I:%M %p")
            cur_loc = dictevs[i][1]
            cur_sum = dictevs[i][2]

            ts_output = i.strftime("%_I:%M %p")
            print(ts_output + " to " + te_output + ": " + cur_sum.rstrip() + " " + "{{" + cur_loc.rstrip() + "}}")
            first_el += 1
            prev_date = i.date()
    
    return


"""
    The code below configures the argparse module for use with
  assignment #2.
""" 
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--file', type=str, help='file to be processed')
    parser.add_argument('--start', type=str, help='start of date range')
    parser.add_argument('--end', type=str, help='end of data range')

    args = parser.parse_args()
    """
    print ('file: {}; start: {}; end: {}'.format( args.file, args.start,
        args.end))
    """
    if not args.file:
        print("Need --file=<ics filename>")

    if not args.start:
        print("Need --start=dd/mm/yyyy")

    if not args.end:
        print("Need --end=dd/mm/yyyy")
    

    """ Make start and endtime into objects of type datetime """
    sy,sm,sd = args.start.split("/")
    start_date = datetime.date(int(sy),int(sm),int(sd))
    ey,em,ed = args.end.split("/")
    end_date = datetime.date(int(ey),int(em),int(ed))

    """ Open the file, split the iCal file into a dictionary, then print the events and close the file """
    f = open(args.file, "r")
    dofe = dict_events(f)
    f.close()
    print_events(dofe,start_date,end_date)


if __name__ == "__main__":
    main()
