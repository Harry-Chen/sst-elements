import sst

# Define SST core options
sst.setProgramOption("timebase", "1ps")
sst.setProgramOption("stopAtCycle", "0 ns")

num_cpus = 16
comp_cpu = [None] * num_cpus
comp_l1cache = [None] * num_cpus

for i in range(num_cpus):
    # Define the simulation components
    comp_cpu[i] = sst.Component("cpu{}".format(i), "miranda.BaseCPU")
    comp_cpu[i].addParams({
            "verbose" : 0,
            "printStats" : 1,
    })

    # Put the miranda.CopyGenerator subcomponent into comp_cpu's 'generator' slot
    gen = comp_cpu[i].setSubComponent("generator", "miranda.CopyGenerator")
    gen.addParams({"verbose" : 0,
        "read_start_address" : 0,
        "request_width" : 16,
        "request_count" : 65536,
    })
    # Enable statistics outputs
    comp_cpu[i].enableAllStatistics({"type":"sst.AccumulatorStatistic"})

    comp_l1cache[i] = sst.Component("l1cache{}".format(i), "memHierarchy.Cache")
    comp_l1cache[i].addParams({
          "access_latency_cycles" : "2",
          "cache_frequency" : "2 Ghz",
          "replacement_policy" : "lru",
          "coherence_protocol" : "MESI",
          "associativity" : "4",
          "cache_line_size" : "64",
          "prefetcher" : "cassini.StridePrefetcher",
          "debug" : "1",
          "L1" : "1",
          "cache_size" : "2KB"
    })

    # Enable statistics outputs
    comp_l1cache[i].enableAllStatistics({"type":"sst.AccumulatorStatistic"})

# Tell SST what statistics handling we want
sst.setStatisticLoadLevel(4)


# Define the simulation links
for i in range(num_cpus):

    comp_memory = sst.Component("memory{}".format(i), "memHierarchy.MemController")
    comp_memory.addParams({
          "coherence_protocol" : "MESI",
          "backend.access_time" : "1000 ns",
          "backend.mem_size" : "512MiB",
          "clock" : "1GHz"
    })

    link_cpu_cache_link = sst.Link("link_cpu_cache_link{}".format(i))
    link_cpu_cache_link.connect( (comp_cpu[i], "cache_link", "1000ps"), (comp_l1cache[i], "high_network_0", "1000ps") )
    link_cpu_cache_link.setNoCut()

    link_mem_bus_link = sst.Link("link_mem_bus_link{}".format(i))
    link_mem_bus_link.connect( (comp_l1cache[i], "low_network_0", "50ps"), (comp_memory, "direct_link", "50ps") )

