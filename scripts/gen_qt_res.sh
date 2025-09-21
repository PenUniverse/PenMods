echo 'generating resource.qrc...'

script_path=$(pwd)

cd ../resource/models/$1
python $script_path/gen_qrc.py . resource.qrc

echo 'generating qrc_qml.h...'

rcc -name qml resource.qrc -o qrc_qml.cpp

Temp=$(sed -n '$=' qrc_qml.cpp)
sed $(($Temp-58+1)),${Temp}d -i qrc_qml.cpp

mv qrc_qml.cpp qrc_qml.h

echo 'done.'
