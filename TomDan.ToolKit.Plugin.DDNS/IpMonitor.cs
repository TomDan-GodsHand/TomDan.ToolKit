using System;

namespace TomDan.ToolKit.Plugin.DDNS;


    public class IpMonitor
    {
        public IPAddress CurrentIp { get; set; }
        public DateTime LastTime { get; set; }
        public TimeSpan CheckFrequency { get; set; }
        public DDNS DDNS { get; set; }

        public IpMonitor(DDNS ddns)
        {
            var TaskCurrentIp = GetRecordIp(ddns);
            CurrentIp = TaskCurrentIp.Result;
            LastTime = DateTime.Now;
            CheckFrequency = TimeSpan.FromSeconds(5);
            DDNS = ddns;
        }

        public async void MainLoop()
        {
            using var timer = new PeriodicTimer(CheckFrequency);

            while (await timer.WaitForNextTickAsync())
            {
                int count = 10;

                // First loop: Check IP
                while (count > 0)
                {
                    try
                    {
                        bool checkRes = await CheckIp();
                        if (checkRes)
                        {
                            break;
                        }
                        else
                        {
                            count--;
                            await Task.Delay(TimeSpan.FromSeconds(10));
                            Console.WriteLine($"检查失败, 正在重试第 {count} 次");
                        }
                    }
                    catch { }
                }

                // Second loop: Get current record and update if necessary
                count = 10;
                while (count > 0)
                {
                    var recordItem = await DDNS.GetCurrentRecord();

                    if (recordItem != null)
                    {
                        if (recordItem.Value != CurrentIp.ToString())
                        {
                            var res = await DDNS.ChangeRecord(recordItem,CurrentIp.ToString());

                            if (res)
                            {
                                Console.WriteLine($"更新 IP 状态: {res}");
                            }
                            else
                            {
                                Console.WriteLine("更新失败");
                            }
                        }
                        break;
                    }
                    else
                    {
                        count--;
                        await Task.Delay(TimeSpan.FromSeconds(10));
                        Console.WriteLine($"检查失败, 正在重试第 {count} 次");
                    }
                }
            }
        }
        private async Task<IPAddress?> GetRecordIp(DDNS ddns)
        {
            try
            {
                var recordItem = await ddns.GetCurrentRecord();
                return recordItem switch
                {
                    { Value: var value } when IPAddress.TryParse(value, out IPAddress ip) && ip.AddressFamily == System.Net.Sockets.AddressFamily.InterNetworkV6
                        => ip,
                    _ => IPAddress.IPv6Loopback
                };
            }
            catch (Exception)
            {
                // Handle the exception and return default IPv6 loopback address
                return IPAddress.IPv6Loopback;
            }
        }
        public async Task<string> GetIpAsync()
        {
            try
            {
                using var httpClient = new HttpClient();
                var response = await httpClient.GetAsync("https://6.ipw.cn");

                response.EnsureSuccessStatusCode();
                var a =await response.Content.ReadAsStringAsync();
                return a.Trim(); // Trimming in case of extra spaces or new lines
            }
            catch (Exception ex)
            {
                throw new Exception("Failed to retrieve IP address", ex);
            }
        }

        private async Task<bool> CheckIp()
        {

            try
            {
                string ip = await GetIpAsync();
                if (ip != CurrentIp.ToString())
                {
                    Console.WriteLine($"IP 不同，开始更新，旧 IP: {CurrentIp}, 新 IP: {ip}");
                    CurrentIp = System.Net.IPAddress.Parse(ip);
                    LastTime = DateTime.Now;
                }
                else
                {
                    Console.WriteLine("IP 相同，时间未变");
                }
                return true;
            }
            catch (Exception)
            {
                Console.WriteLine("获取 IP 失败，可能不在 IPv6 网络中");
                return false;
            }
        }

    }
}