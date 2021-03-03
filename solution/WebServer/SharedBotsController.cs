using Core;
using EmbedIO.WebApi;
using System;
using System.Collections.Generic;
using System.Text;

namespace WebServer
{
    class SharedBotsController : WebApiController
    {
        private readonly IDispatcher mDipatcher;
        public SharedBotsController(IDispatcher dispatcher)
        {
            mDipatcher = dispatcher;
        }
    }
}
