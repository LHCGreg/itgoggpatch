#!/usr/local/bin/python

import subprocess
import re
import sys

localCopyDir = sys.argv[1]
print 'local copy dir = ' + localCopyDir
templateFilePath = sys.argv[2]
print 'template file path = ' + templateFilePath
outputFilePath = sys.argv[3]
print 'output file path = ' + outputFilePath

templateFile = open(templateFilePath, 'r')
templateContents = templateFile.read()
templateFile.close()

versionProgram = 'svnversion'
try:
    svnversionArgs = [versionProgram, localCopyDir, '-c']
    svnversion = subprocess.Popen(svnversionArgs, stdout=subprocess.PIPE)
    revisionString = svnversion.communicate()[0]
    print 'revision string = ' + revisionString

    regexMatch = re.match(r"^(?:.*:)?(\d+)", revisionString)
    if regexMatch:
        revision = regexMatch.group(1)
        print 'revision = ' + revision
    else:
        print "Couldn't find revision number, using 0."
        revision = '0'
        
    print revision
except OSError:
    revision = '0'
    print 'The ' + versionProgram + ' program could not be found. Using 0 as the revision number. You should have the SVN client tools installed and on your path. If you are using TortoiseSVN, you must still download the SVN command-line client.'
        

processedTemplate = templateContents.replace('$revision$', revision)
output = open(outputFilePath, 'w')
output.write(processedTemplate)
output.close()

print 'Version template replacement successful'
