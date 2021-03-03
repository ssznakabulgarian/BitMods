using Core;
using EmbedIO.WebApi;
using System;
using System.Collections.Generic;
using System.Text;

namespace WebServer
{
    class ProgramLibraryController : WebApiController
    {
        private readonly IDispatcher mDipatcher;
        public ProgramLibraryController(IDispatcher dispatcher)
        {
            mDipatcher = dispatcher;
        }
    }
}
