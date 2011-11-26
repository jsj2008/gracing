#!/bin/bash

# reference
# http://stackoverflow.com/questions/96882/how-do-i-create-a-nice-looking-dmg-for-mac-os-x-using-command-line-tools

function executeAndCheck {
	$*
  if [ $? -ne 0 ]; then
		echo "Failure"
	  exit
	fi
}


source=../gracing.app
title=gracing
size=40000
tempname=pack.temp.dmg
finalDMGName=gracing.dmg

#executeAndCheck false
echo "Creating temp dist '${tempname}' from '${source}' (size: ${size})"
hdiutil create -srcfolder "${source}" -volname "${title}" -fs HFS+ \
      -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${size}k ${tempname}


echo "Attaching...."
device=$(hdiutil attach -readwrite -noverify -noautoopen "${tempname}" | \
         egrep '^/dev/' | sed 1q | awk '{print $1}')
echo "done (device obtained ${device})"


echo "Operaing with action script"
echo '
   tell application "Finder"
     tell disk "'${title}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {400, 100, 885, 430}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 72
           set background picture of theViewOptions to file ".background:'${backgroundPictureName}'"
           make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
           set position of item "'${applicationName}'" of container window to {100, 100}
           set position of item "Applications" of container window to {375, 100}
           update without registering applications
           delay 5
           eject
     end tell
   end tell
' | osascript
exit
chmod -Rf go-w /Volumes/"${title}"
sync
sync
hdiutil detach ${device}
hdiutil convert "/${tempname}" -format UDZO -imagekey zlib-level=9 -o "${finalDMGName}"
rm -f /${tempname}

