using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ComSupport
{
    /** Base-class for all COM classes. Handles object ref-counting automatically. */
    public class ComClass
    {
        public static int m_obj_cnt = 0;

        public ComClass()
        {
            Interlocked.Increment(ref m_obj_cnt);
        }

        ~ComClass()
        {
            Interlocked.Decrement(ref m_obj_cnt);
        }
    }
}
