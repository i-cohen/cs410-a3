#!/usr/bin/perl

use CGI qw(:standard);

print "Content-type:text/html\r\n\r\n";
print "<html>\n";
print "<head>\n";
print "<title>Table of GET args</title>\n";
print "</head>\n";
print "<body>\n";
print "<h2>Table of GET args</h2>\n";
print "<h3>";

print "<table>";
for my $name (CGI::param()) {
    for my $val (CGI::param($name)) {
    	print "<tr>";
        print "<td> name: $name </td>\n";
        print "<td> value: $val </td>\n";
        print "</tr>";
    }
}
print "</table>";

print "<h3>\n";
print "</body>\n";
print "</html>\n";

1;
