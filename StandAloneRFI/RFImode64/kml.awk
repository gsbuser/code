BEGIN{

    # valid colours: blue, red, green, yellow, orange, pink, purple
    # can use e.g., -v colour=green

    print "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    print "<kml xmlns=\"http://earth.google.com/kml/2.2\">"
      }

{
  if ($4) name=$4;
  else name=NR;

    coordstr=$3 # in format '@lat,long'
      sub("@","",coordstr) # remove the @ sign
      split(coordstr, coord, ",") # split into array
    lat=coord[1]
    lon=coord[2]
    print "<Placemark>"
    print "  <name>"name"</name>"
    print "  <description>"lat","lon" - "FILENAME"</description>"
    print "  <Point>"
      if (colour) print "<Style><IconStyle><Icon><href>http://www.google.com/intl/en_us/mapfiles/ms/icons/"colour"-dot.png</href></Icon></IconStyle></Style>"
    print "    <coordinates>"lon","lat"</coordinates>"
    print "  </Point>"
    print "</Placemark>"
    print ""
}

END {
    print "</kml>"
      }
