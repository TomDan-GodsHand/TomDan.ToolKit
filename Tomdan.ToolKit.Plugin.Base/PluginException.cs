using System;

namespace Tomdan.ToolKit.Plugin.Base;

public class PluginException : Exception
{
    public PluginException(string? message) : base(message)
    {
    }
}
