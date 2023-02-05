namespace DaikinAPI.API
{
    public class ReceivedDaikinState
    {
        public int Timestamp { get; set; }

        public int Temperature { get; set; }

        public int Fan { get; set; }

        public int PowerState { get; set; }

        public int Swing { get; set; }

        public int TimerOn { get; set; }

        public int TimerOnValue { get; set; }

        public int TimerOff { get; set; }

        public int TimerOffValue { get; set; }

        public int Mode { get; set; }
    }
}
