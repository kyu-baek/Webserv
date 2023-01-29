
#!/usr/bin/perl -w
use CGI;


while (<STDIN>) {
    print ;
}

$upload_dir = "/Users/kbaek/42seoul/webserv/database";


$query = new CGI;

$filename = $query->param("filename");
$filename =~ s/.*[\/\\](.*)/$1/;
$upload_filehandle = $query->upload("filename");

open UPLOADFILE, ">$upload_dir/$filename";

while ( <$upload_filehandle> )
{
    print UPLOADFILE;
}

close UPLOADFILE;


# print "Status: 200 OK\r\n";
# print $query->header ( );
# print <<END_HTML;
# <HTML>
# <HEAD>
# <TITLE>Thanks!</TITLE>
# </HEAD>
# <BODY>
# <P>Thanks for uploading your file!</P>
# <P>Your file: $upload_dir/$filename</P>
# </BODY>
# </HTML>
# END_HTML
