using System;
using System.Net;
using System.Text;
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
        UserNames = new();
    }

    public Dictionary<string, string> UserNames { get; set; }
    public void ConfigMqttServer(string ipAddress, int port)
    {
        mqttServerOption = new MqttServerOptionsBuilder().WithDefaultEndpoint()
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
            mqttServer.ClientConnectedAsync += ClientConnectedAsync;
            mqttServer.ClientSubscribedTopicAsync += ClientSubscribedTopicAsync;
            mqttServer.ClientUnsubscribedTopicAsync += ClientUnsubscribedTopicAsync;
            mqttServer.ClientDisconnectedAsync += OnClientDisConnected;
            mqttServer.InterceptingPublishAsync += OnInterceptionPublish;
            mqttServer.ApplicationMessageNotConsumedAsync += ApplicationMessageNotConsumedAsync;
            await mqttServer.StartAsync();
            return true;
        }
        catch (Exception ex)
        {
            throw ex;
        }
    }

    private Task ClientConnectedAsync(ClientConnectedEventArgs args)
    {
        logger.LogInformation($"Client '{args.ClientId}' connected.");
        return Task.CompletedTask;
    }

    private async Task Validation_ConnectionAsync(ValidatingConnectionEventArgs e)
    {
        logger.LogInformation($"Client '{e.ClientId}' wants to connect by '{e.UserName}'");
        if (!UserNames.ContainsKey(e.UserName))
        {
            logger.LogInformation($"Client '{e.ClientId}' Rejected");
            e.ReasonCode = MqttConnectReasonCode.BadUserNameOrPassword;
            return;
        }
        if (UserNames[e.UserName] != e.Password)
        {
            logger.LogInformation($"Client '{e.ClientId}' Rejected");
            e.ReasonCode = MqttConnectReasonCode.BadUserNameOrPassword;
            return;
        }
        logger.LogInformation($"Client '{e.ClientId}' Accepted");
    }
    public async Task ApplicationMessageNotConsumedAsync(ApplicationMessageNotConsumedEventArgs e)
    {
        var Topic = e.ApplicationMessage.Topic;
        logger.LogInformation($"消息未消费{Topic}:");
    }

    public async Task ClientSubscribedTopicAsync(ClientSubscribedTopicEventArgs e)
    {
        logger.LogInformation($"客户端{e.ClientId}订阅主题:{e.TopicFilter.Topic}");
    }

    public async Task ClientUnsubscribedTopicAsync(ClientUnsubscribedTopicEventArgs e)
    {
        logger.LogInformation($"客户端{e.ClientId} 取消主题订阅:{e.TopicFilter}");
    }

    public async Task OnClientDisConnected(ClientDisconnectedEventArgs args)
    {
        logger.LogInformation($"客户端下线 id:{args.ClientId}");
    }

    public async Task OnInterceptionPublish(InterceptingPublishEventArgs e)
    {
        var msg = Encoding.UTF8.GetString(e.ApplicationMessage.Payload);
        var Topic = e.ApplicationMessage.Topic;
        logger.LogInformation($"客户端{e.ClientId} 主题{Topic} 发送消息 内容:");
        logger.LogInformation(msg);
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
