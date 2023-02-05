namespace DaikinAPI.Model
{
    public class DaikinState
    {
        public int Timestamp { get; set; }

        public int Temperature { get; set; }

        public int Fan { get; set; }

        public bool IsOn { get; set; }

        public bool IsSwinging { get; set; }

        public int TimerOn { get; set; }

        public int TimerOnValue { get; set; }

        public int TimerOff { get; set; }

        public int TimerOffValue { get; set; }

        public int Mode { get; set; }
    }
}
