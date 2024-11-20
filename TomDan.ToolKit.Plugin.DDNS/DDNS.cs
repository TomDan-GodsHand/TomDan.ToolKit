using System;
using Microsoft.Extensions.Logging;

namespace TomDan.ToolKit.Plugin.DDNS
{

    public class DDNS
    {
        private string domain { set; get; }
        private RequestClient requestClient { set; get; }
        private string subDomain { set; get; }
        public ILogger<DDNS> logger { get; set; }
        public DDNS(string secretId, string secretKey, string domain, string subDomain, ILogger<DDNS> logger)
        {
            string host = "dnspod.tencentcloudapi.com";
            string service = "dnspod";
            requestClient = new RequestClient(secretId, secretKey, host, service);
            this.logger = logger;
        }

        public async Task<(bool, DescribeRecordListResponse?)> QueryRecordList(string recordType)
        {
            try
            {
                var (success, response) = await requestClient.Send<DescribeRecordList, DescribeRecordListResponse>(
                    "DiscribeRecordList",
                    new() { Domain = domain, RecordType = recordType });
                if (success)
                {
                    return (success, response.response);
                }
                else
                {
                    throw new Exception("获取失败");
                }
            }
            catch (Exception ex)
            {
                throw ex;
            }
        }
        public async Task<RecordListItem?> GetCurrentRecord() => await QueryRecordList("AAAA") switch
        {
            (false, null) => throw new Exception("获取失败"),  // 如果查询失败或返回 null
            (true, { RecordList: { Count: > 0 } recordList }) => recordList.LastOrDefault(),  // 成功并且有记录
            _ => throw new Exception("获取失败")
        };

        public async Task<RecordListItem> QueryRecordByName()
        {
            try
            {

                var (success, RecordList) = await QueryRecordList(string.Empty);
                if (!success || RecordList == null || RecordList.RecordList.Count == 0)
                    throw new Exception("查询列表失败");

                var res = RecordList.RecordList.FirstOrDefault(x => x.Name == this.subDomain)
                          ?? throw new Exception("未找到，先去手动绑定!");
                return res;

            }
            catch { throw; }

        }

        public async Task<bool> ChangeRecord(RecordListItem recordItem, string value)
        {
            try
            {
                var (success, res) = await requestClient.Send<ModifyRecordRequest, ModifyRecordResponse>("ModifyRecord", new ModifyRecordRequest()
                {
                    Value = value,
                    Domain = this.domain,
                    RecordType = recordItem.Type,
                    RecordLine = recordItem.Line,
                    RecordId = recordItem.RecordId,
                    SubDomain = recordItem.Name,  // 使用值，不是 Some/None
                    DomainId = null,
                    RecordLineId = null,
                    MX = null,
                    TTL = null,
                    Weight = null,
                    Status = null
                });
                if (success)
                    return true;
                else
                    return false;
            }
            catch (Exception ex)
            {
                throw;
            }
        }

        public async void ChangeRecordLoop(string currentIp)
        {
            int count = 10;
            while (true)
            {
                try
                {
                    var result = await QueryRecordByName();

                    if (result.Value != currentIp)
                    {
                        var changeResult = await ChangeRecord(result, currentIp);

                        if (changeResult)
                        {
                            logger.LogError($"发现 IP 变化，解析时间：current_time:{DateTime.Now}");
                        }
                        else
                        {
                            logger.LogError("更新 IP 状态失败");
                        }
                    }
                    else
                    {
                        logger.LogError("IP 未发生变化");
                    }
                    break;
                }
                catch (Exception ex)
                {
                    count--;
                    await Task.Delay(TimeSpan.FromSeconds(10));
                    logger.LogError($"检查失败，正在重试第 {count} 次");

                    if (count == 0)
                    {
                        break;
                    }
                }
            }
        }
    }
}