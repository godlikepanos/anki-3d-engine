# Set the environment variables of the Visual Studio native prompt

$path1="C:\opt\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
$path2="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

if(Test-Path $path1) {
  $path=$path1
}
elseif(Test-Path $path2) {
  $path=$path2
}
else {
  echo "vcvars64.bat not found"
  exit
}

echo "Will call `"$path`""

cmd.exe /c "call `"$path`" && set > %temp%\vcvars.txt"

Get-Content "$env:temp\vcvars.txt" | Foreach-Object {
  if($_ -match "^(.*?)=(.*)$") {
    Set-Content "env:\$($matches[1])" $matches[2]
  }
}
