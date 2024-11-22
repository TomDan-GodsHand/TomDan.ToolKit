# 读取配置文件
$configPath = ".vscode\\plugin-config.json"
$config = Get-Content $configPath | ConvertFrom-Json

# 遍历插件配置并执行复制任务
foreach ($plugin in $config.plugins) {
    $source = $plugin.source
    $destination = $plugin.destination

    # 检查源目录是否存在
    if (Test-Path $source) {
        Write-Host "Copying from $source to $destination"
        Copy-Item -Path $source\* -Destination $destination -Recurse -Force
    }
    else {
        Write-Host "Source directory $source does not exist!"
    }
}
