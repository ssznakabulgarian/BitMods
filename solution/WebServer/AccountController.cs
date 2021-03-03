using Core;
using EmbedIO.WebApi;
using System;
using System.Collections.Generic;
using System.Text;

namespace WebServer
{
    class AccountController : WebApiController
    {
        private readonly IDispatcher mDipatcher;
        public AccountController(IDispatcher dispatcher)
        {
            mDipatcher = dispatcher;
        }
    }
}
