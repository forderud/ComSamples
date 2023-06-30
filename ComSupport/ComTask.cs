using System;
using System.Threading;
using System.Threading.Tasks;

namespace ComSupport
{
    public class ComTask
    {
        /** Task with the thread initialized in a specified COM apartment.
         *   Enables communication over AppAPI and other COM interfaces. */
        public static Task<T> Run<T>(ApartmentState apartment, Func<T> func)
        {
            var tcs = new TaskCompletionSource<T>();
            Thread thread = new Thread(() => {
                try
                {
                    tcs.SetResult(func());
                }
                catch (Exception e)
                {
                    tcs.SetException(e);
                }
            });
            thread.SetApartmentState(apartment);
            thread.Start();
            return tcs.Task;
        }
    }
}
