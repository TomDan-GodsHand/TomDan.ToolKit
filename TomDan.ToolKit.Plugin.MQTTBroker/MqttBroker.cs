using System;
using System.Net;
using Microsoft.Extensions.Logging;
using MQTTnet.Protocol;
using MQTTnet.Server;

namespace TomDan.ToolKit.Plugin.MQTTBroker;

public class MqttBroker
{
    private MqttServer mqttServer;

    public MqttBroker(ILogger<MqttBroker> logger)
    {
        this.logger = logger;
    }

    public Dictionary<string, string> UserNames { get; set; }
    public void ConfigMqttServer(string ipAddress, int port)
    {
        mqttServerOption = new MqttServerOptionsBuilder().WithDefaultEndpointBoundIPAddress(IPAddress.Parse(ipAddress))
                                                         .WithDefaultEndpointPort(port)
                                                         .Build();
    }
    public void AddUser(string username, string password)
    {
        UserNames.Add(username, password);
    }
    public async Task<bool> StartMqtt()
    {
        try
        {
            if (mqttServerOption == null) throw new Exception("请先配置");
            mqttServer = new MqttServerFactory().CreateMqttServer(mqttServerOption);
            mqttServer.ValidatingConnectionAsync += Validation_ConnectionAsync;
            await mqttServer.StartAsync();
            return true;
        }
        catch (Exception ex)
        {
            throw ex;
        }
    }

    private async Task Validation_ConnectionAsync(ValidatingConnectionEventArgs e)
    {
        if (!UserNames.ContainsKey(e.UserName))
        {
            e.ReasonCode = MqttConnectReasonCode.BadUserNameOrPassword;
        }
        if (UserNames[e.UserName] != e.Password)
        {
            e.ReasonCode = MqttConnectReasonCode.BadUserNameOrPassword;
        }

    }

    public async Task<bool> StopAsync()
    {
        try
        {
            if (mqttServer == null) throw new Exception("不存在MqttServer");
            await mqttServer.StopAsync();
            return true;
        }
        catch (Exception ex)
        {
            throw ex;
        }
    }
    public ILogger<MqttBroker> logger { get; }
    public MqttServerOptions mqttServerOption { get; private set; }
}
