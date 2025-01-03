using System;
using System.Text.Json.Serialization;

namespace TomDan.ToolKit.Plugin.DDNS
{
    public class ModifyRecordRequest
    {
        /// <summary>
        /// 域名
        /// </summary>
        public string Domain { get; set; }

        /// <summary>
        /// 记录类型，通过 API 记录类型获得，大写英文，比如：A 。
        /// </summary>
        public string RecordType { get; set; }

        /// <summary>
        /// 记录线路，通过 API 记录线路获得，中文，比如：默认。
        /// </summary>
        public string RecordLine { get; set; }

        /// <summary>
        /// 记录值，如 IP : 200.200.200.200， CNAME : cname.dnspod.com.， MX : mail.dnspod.com.。
        /// </summary>
        public string Value { get; set; }

        /// <summary>
        /// 记录 ID 。可以通过接口DescribeRecordList查到所有的解析记录列表以及对应的RecordId
        /// </summary>
        public long RecordId { get; set; }

        /// <summary>
        /// 主机记录，如 www，如果不传，默认为 @。
        /// </summary>
        [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
        public string SubDomain { get; set; }


    }

    public class DescribeRecordList
    {
        public string Domain { get; set; }

        [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
        public string RecordType { get; set; }
    }

    public class RecordListItem
    {
        /// <summary>
        /// 记录Id
        /// </summary>
        public long RecordId { get; set; }

        /// <summary>
        /// 记录值
        /// </summary>
        public string Value { get; set; }

        /// <summary>
        /// 主机名
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// 记录类型
        /// </summary>
        public string Type { get; set; }

        /// <summary>
        /// 记录线路
        /// </summary>
        public string Line { get; set; }
    }

    public class TcErrorResponse
    {
        public string Code { get; set; }
        public string Message { get; set; }
    }
    public class TcResponse<T>
    {
        public T response { get; set; }
    }
    public class DescribeRecordListResponse
    {
        public TcErrorResponse Error { get; set; }
        public List<RecordListItem> RecordList { get; set; }
    }

    public class ModifyRecordResponse
    {
        public TcErrorResponse Error { get; set; }
        public long? RecordId { get; set; }
    }
}