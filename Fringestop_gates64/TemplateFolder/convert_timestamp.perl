# Converts a list of timestamps into TJD's.
# TJD = JD - 2440000.5
#
# Input format is:
# Fri Jul  1 12:35:00 CST 2005
#
# The input is read from standard input, the output goes to standard output.
#
# Method here does not currently work for year<2001, this would require
# some modification.  For 21CMA this does not seem to be a major issue :)

# Day of year on which each month starts
%doy_noleap = (  'Jan' =>   0,
                 'Feb' =>  31,
                 'Mar' =>  59,
                 'Apr' =>  90,
                 'May' => 120,
                 'Jun' => 151,
                 'Jul' => 181,
                 'Aug' => 212,
                 'Sep' => 243,
                 'Oct' => 273,
                 'Nov' => 304,
                 'Dec' => 334
);
%doy_leap   = (  'Jan' =>   0,
                 'Feb' =>  31,
                 'Mar' =>  60,
                 'Apr' =>  91,
                 'May' => 121,
                 'Jun' => 152,
                 'Jul' => 182,
                 'Aug' => 213,
                 'Sep' => 244,
                 'Oct' => 274,
                 'Nov' => 305,
                 'Dec' => 335
);

# Get first date
$line = <>;

  # Get parts of date
  ($dow, $mon, $day, $hms, $zone, $year, $sec_offset) = split ' ', $line;
  ($hour, $min, $sec) = split ':', $hms;
  $sec += $sec_offset;

  # Construct non-integer day
  $day_ni = $day + $hour/24.0 + $min/1440.0 + $sec/86400.0;

  # Add time zone correction to get to UTC
  $day_ni -=  8.0/24.0 if ($zone eq 'CST');
  $day_ni -=  5.5/24.0 if ($zone eq 'IST');
  $day_ni +=  5.0/24.0 if ($zone eq 'EST');
  $day_ni +=  4.0/24.0 if ($zone eq 'EDT');

  # Is this a leap year?
  $is_leapyear = 0;
  $is_leapyear = 1 if ($year%4==0);
  $is_leapyear = 0 if ($year%100==0);
  $is_leapyear = 1 if ($year%400==0);

  # Get day of year
  if ($is_leapyear==1) {
    $doy = $day_ni + $doy_leap{$mon};
  } else {
    $doy = $day_ni + $doy_noleap{$mon};
  }

  # Now for the TJD.
  $trunc_year = $year - 2001;
  if ($trunc_year < 0) {die "Error: year $year\<2001\n";}
  $TJD0 = 11909 + 365*$trunc_year
          + ($trunc_year-($trunc_year%  4))/  4
          - ($trunc_year-($trunc_year%100))/100
          + ($trunc_year-($trunc_year%400))/400
          + $doy;

while ($line = <>) {
  $TJD = $TJD0 + $line/86400.0;
  print (sprintf "%14.8f\n", $TJD);
}
