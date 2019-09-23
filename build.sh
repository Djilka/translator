#!/bin/bash

NAME="test"
SRC=" *.c "${NAME}".cpp"
rm -rf *.class *.so *.c *.h gen

DIR_TEST="jni"

echo ${SRC}
echo ${NAME}

g++ -o ${DIR_TEST}/gen -std=c++17 generator.cpp main.cpp ;

cd ${DIR_TEST};

./gen ${NAME}.hpp ;

g++ -fPIC -std=c++17 -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -shared -o libso.so ${SRC} ;

javac example.java ;

java -Djava.library.path=.  example ;

cd -;
