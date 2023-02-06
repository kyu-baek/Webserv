#!/usr/bin/perl -w
use CGI;
use Env;

$upload_dir = $ENV{'UPLOAD_PATH'};

$query = new CGI;

$filename = $query->param("file");
$filename =~ s/.*[\/\\](.*)/$1/;
$upload_filehandle = $query->upload("file");

open UPLOADFILE, ">$upload_dir/$filename";

while ( <$upload_filehandle> )
{
    print UPLOADFILE;
}

close UPLOADFILE;

# print "Status: 200 OK\r\n";
# print $query->header ( );

print <<END_HTML;
<HTML>
<HEAD>
<TITLE>Thanks!</TITLE>
</HEAD>
<BODY>
<div><a href="/home">Go to home</a></div>
<P>Thanks for uploading your file!</P>
<P>Your file: Dir = $upload_dir </p> <p> File = $filename</P>
</BODY>
</HTML>
END_HTML
