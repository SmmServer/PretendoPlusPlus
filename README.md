# Pretendo++

Super dumb account server returning static data, just enough to satisfy Cemu.

Related repositories:

- https://github.com/mrexodia/SMMNEX
- https://github.com/mrexodia/NintendoClients/tree/friends-server
- https://github.com/Stary2001/nex-dissector

## Fiddler

Rules -> Customize Rules... at the end of the `OnBeforeRequest` method add the following:

```
if (oSession.HostnameIs("account.nintendo.net"))
{
    if (oSession.HTTPMethodIs("CONNECT"))
    {
        // This is just a fake tunnel for CONNECT requests
        oSession["x-replywithtunnel"] = "PretendoTunnel";
        return;
    }
    oSession.fullUrl = "http://127.0.0.1:8383" + oSession.PathAndQuery;
}
```

Next up, export the Fiddler root certificate:

- In Fiddler, open Tools > Options
- In the HTTPS tab turn on HTTPS Connects
- Enable HTTPS decrypting
- Ignore server certificate errors
- In the Connections tab tick Allow remote computers to connect
- Turn off Act as system proxy on startup
- Back in the HTTPS tab click Actions > Export Root Certificate to Desktop, which will give you `FiddlerRoot.cer`

As long as you don't force Cemu/WiiU to connect to Fiddler as a proxy you will not interfere with online services.

## Cemu

You can use the `CemuMagic` module on **Cemu 1.15.3b** from this repository: https://github.com/mrexodia/AppInitHook/tree/cemu

Inject CemuMagic.dll into Cemu.exe before startup (either by using the AppInitHook framework, or by other methods).

You need to copy your `FiddlerRoot.cer` into `Cemu\BIN\mlc01\sys\title\0005001b\10054000\content\scerts\` with the following names:

```
CACERT_NINTENDO_CA.der.proxy
CACERT_NINTENDO_CA_G2.der.proxy
CACERT_NINTENDO_CA_G3.der.proxy
CACERT_NINTENDO_CLASS2_CA.der.proxy
CACERT_NINTENDO_CLASS2_CA_G2.der.proxy
CACERT_NINTENDO_CLASS2_CA_G3.der.proxy
```

You also have to set Cemu up to work online. This method will not interfere with online services if you are not injecting `CemuMagic.dll`.

You can use [DebugView](https://docs.microsoft.com/en-us/sysinternals/downloads/debugview) to see the logs for `CemuMagic`.