from lxml import etree
import sys

doc = etree.ElementTree( file=sys.argv[ 1 ] )
root = doc.getroot()

for annotation in root.iter( "annotation" ):
    output = '"{0}": '.format( annotation[0][0].text )
    for rect in annotation.iter( "annorect" ):
        output += "({0}, {1}, {2}, {3}), ".format(rect[0].text,rect[1].text,rect[2].text,rect[3].text)
    output = output[:-2] + ";" 
    print output

