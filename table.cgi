#!/usr/bin/perl

use CGI qw(:standard);

print "Content-type:text/html\r\n\r\n";
print "<html>\n";
print "<head>\n";
print "<meta charset=\"UTF-8\">";
print "<title>Table of GET args</title>\n";
print "</head>\n";
print "<body style=\"background-color: lightblue;\">\n";
print "<h2 style=\"text-align: center\">Table of GET args</h2>\n";
print "<h3>";

print "<table style=\"display: block; margin-left: auto; margin-right: auto; width:60%;\">";
print "<tr>";
print     "<th>Key</th>";
print     "<th>Value</th>";
print "</tr>";
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
print "<style>\n";
print "table, th, td {\n";
print "    border: 1px solid black;\n";
print "}\n";
print "</style>\n";
print "</html>\n";

1;
