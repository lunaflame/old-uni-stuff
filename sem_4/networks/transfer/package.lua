return {
  name = "nw_lab2",
  version = "0.1",
  private = true, -- This prevents us from accidentally publishing this package.
  dependencies = {
    "luvit/require",
    "luvit/fs",
    "luvit/dns",
    "luvit/path",
    "luvit/process"
  },
}