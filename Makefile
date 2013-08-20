
onshutdown.exe: onshutdown.c onshutdown.res onshutdown.def
    gcc -Zomf -Zsys -O2 -Zc++-comments -o $@ $**

onshutdown.res: onshutdown.rc onshutdown.ico
    rc -r $*.rc $@

