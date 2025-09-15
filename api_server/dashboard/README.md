# net

A new Flutter project.

$env:PUB_HOSTED_URL="https://mirror.sjtu.edu.cn/dart-pub"
$env:FLUTTER_STORAGE_BASE_URL="https://mirror.sjtu.edu.cn"

echo $env:PUB_HOSTED_URL
echo $env:FLUTTER_STORAGE_BASE_URL

flutter clean
flutter pub cache repair