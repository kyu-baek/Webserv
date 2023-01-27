#!/usr/bin/perl -w
use CGI;
use 5.010;

$upload_dir = "/Users/kyu/42project/webserv/www";

$query = new CGI;

$filen = $query->param("filename");
# $filen =~ s/.*[\/\\](.*)/$1/;
$upload_filehandle = $query->upload("filename");

open UPLOADFILE, ">$upload_dir/$filen";

while ( <$upload_filehandle> )
{
    print UPLOADFILE;
}

close UPLOADFILE;

print "Status: 200 OK\r\n";
print $query->header ( );
print <<END_HTML;
<HTML>
<HEAD>
<TITLE>Thanks!</TITLE>
</HEAD>
<BODY>
<P>Thanks for uploading your file!</P>
<P>Your file: $upload_dir/$filen</P>
</BODY>
</HTML>
END_HTML