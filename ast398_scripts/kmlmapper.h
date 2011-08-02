#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  char *name;
  char *description;
  char *style;
  long double point[2];
}
entry;

char *bare = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<kml xmlns=\"http://www.opengis.net/kml/2.2\">\r\n\
<Document>\r\n\
  <name>Grand Daddy Map</name>\r\n\
  <description>Listing of all RfI sources near GMRT.\r\n\
Red are candidates as shown on chimap.\r\n\
Blue are transmit sites from GPS coord.\r\n\
Purple are trasmit sites as shown on chimap.\r\n\
Pink are candidates after adding the difference.\r\n\
Green are verified Pink sites and ready for wireman.\r\n\
Yellow are something not specified above</description>\r\n\
  <Style id=\"style1\">\r\n\
    <IconStyle>\r\n\
      <Icon>\r\n\
        <href>http://maps.gstatic.com/intl/en_in/mapfiles/ms2/micons/red-dot.png</href>\r\n\
      </Icon>\r\n\
    </IconStyle>\r\n\
  </Style>\r\n\
  <Style id=\"style5\">\r\n\
    <IconStyle>\r\n\
      <Icon>\r\n\
        <href>http://maps.gstatic.com/intl/en_in/mapfiles/ms2/micons/green-dot.png</href>\r\n\
      </Icon>\r\n\
    </IconStyle>\r\n\
  </Style>\r\n\
  <Style id=\"style2\">\r\n\
    <IconStyle>\r\n\
      <Icon>\r\n\
        <href>http://maps.gstatic.com/intl/en_in/mapfiles/ms2/micons/ltblue-dot.png</href>\r\n\
      </Icon>\r\n\
    </IconStyle>\r\n\
  </Style>\r\n\
  <Style id=\"style6\">\r\n\
    <IconStyle>\r\n\
      <Icon>\r\n\
        <href>http://maps.gstatic.com/intl/en_in/mapfiles/ms2/micons/yellow-dot.png</href>\r\n\
      </Icon>\r\n\
    </IconStyle>\r\n\
  </Style>\r\n\
  <Style id=\"style3\">\r\n\
    <IconStyle>\r\n\
      <Icon>\r\n\
        <href>http://maps.gstatic.com/intl/en_in/mapfiles/ms2/micons/purple-dot.png</href>\r\n\
      </Icon>\r\n\
    </IconStyle>\r\n\
  </Style>\r\n\
  <Style id=\"style4\">\r\n\
    <IconStyle>\r\n\
      <Icon>\r\n\
        <href>http://maps.gstatic.com/intl/en_in/mapfiles/ms2/micons/pink-dot.png</href>\r\n\
      </Icon>\r\n\
    </IconStyle>\r\n\
  </Style>\r\n\
</Document>\r\n\
</kml>";
