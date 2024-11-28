using System;
using Newtonsoft.Json.Linq;

namespace Tomdan.ToolKit.Plugin.Base;

public class PluginConfig
{
    public Dictionary<string, string> Data { get; set; }

    public void InitConfig(string path)
    {
        Data = new();
        var content = File.ReadAllText(path);
        
        JObject jobject = JObject.Parse(content);
        foreach (var item in jobject){
            Data.Add(item.Key,item.Value?.ToString() ?? string.Empty);
        }
    }
}
