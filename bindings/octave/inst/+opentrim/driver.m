classdef driver < handle
  % opentrim.driver  Wrapper for mcdriver — runs an OpenTRIM simulation.
  %
  % Usage:
  %   d = opentrim.driver(cfg);       % initialise with an opentrim.config
  %   d.exec();                        % run (blocking)
  %   d.exec(@on_progress);            % run with progress callback f(frac)
  %   d.exec(@on_progress, 500);       % custom callback interval (ms)
  %   res = d.info();                  % retrieve an opentrim.info result tree
  %   d.save('out.h5');                % save results to HDF5
  %   d.load('out.h5');                % load results from HDF5
  %   c   = d.config();                % get a copy of the active config
  %   ok  = d.is_running();
  %   d.abort(); d.wait(); d.reset();
  %
  % The progress callback receives a single scalar argument in [0, 1].
  % It is called from the main Octave thread while worker threads run.
  % Keep the callback lightweight to avoid slowing down the simulation.
  %
  % IMPORTANT: any opentrim.info objects derived from this driver hold
  % internal pointers to it.  Always keep the driver alive longer than
  % any info objects derived from it.

  properties (Access = private)
    h_ = uint64(0)
  end

  methods

    function self = driver(cfg)
      % driver(cfg)   — allocate and initialise with opentrim.config cfg
      if nargin == 0
        error("No config given. Usage: d = opentrim.driver(config)");
      else
        self.h_ = __opentrim_driver__('new', cfg.get_handle_());
      end

    end

    function delete(self)

      if self.h_ ~= uint64(0)
        __opentrim_driver__('delete', self.h_);
        self.h_ = uint64(0);
      end

    end

    function exec(self, cb, ms)
      % exec()            — run without a progress callback
      % exec(cb)          — run; call cb(frac) every 1000 ms
      % exec(cb, ms)      — run; call cb(frac) every ms milliseconds
      %
      % Ctrl+C aborts the simulation cleanly.  The interrupt is detected
      % at each callback interval, so response latency equals ms (default 1 s).
      if nargin < 2; cb = []; end
      if nargin < 3; ms = 1000; end
      % A function handle must always be present so feval is called each
      % interval — that is the point where Octave checks for Ctrl+C.
      if ~isa(cb, 'function_handle')
        cb = @(~) [];
      end
      unwind_protect
        __opentrim_driver__('exec', self.h_, cb, uint64(ms));
      unwind_protect_cleanup
        % Reached on error or interrupt before the C++ re-throw path fires.
        __opentrim_driver__('abort', self.h_);
        __opentrim_driver__('wait',  self.h_);
      end_unwind_protect
    end

    function res = info(self)
      % info()  Return an opentrim.info object rooted at the result tree.
      %   The driver must remain alive while the info object is in use.
      res = opentrim.info(self.h_);
    end

    function c = config(self)
      % config()  Return a copy of the driver's active configuration.
      h = __opentrim_driver__('config', self.h_);
      c = opentrim.config(h);
    end

    function save(self, filename)
      % save(filename)  Save simulation state and results to an HDF5 file.
      __opentrim_driver__('save', self.h_, filename);
    end

    function load(self, filename)
      % load(filename)  Load simulation state from an HDF5 file.
      __opentrim_driver__('load', self.h_, filename);
    end

    function ok = is_running(self)
      % is_running()  True if worker threads are active.
      ok = __opentrim_driver__('is_running', self.h_);
    end

    function abort(self)
      % abort()  Signal the running simulation to stop early.
      __opentrim_driver__('abort', self.h_);
    end

    function wait(self)
      % wait()  Block until a running simulation finishes.
      __opentrim_driver__('wait', self.h_);
    end

    function n = ion_count(self)
      % ion_count()  Return the current number of simulated ions.
      n = __opentrim_driver__('ion_count', self.h_);
    end

    function n = max_no_ions(self)
      % max_no_ions()  Return the configured maximum number of ions.
      n = __opentrim_driver__('max_no_ions', self.h_);
    end

    function setMaxNoIons(self, n)
      % setMaxNoIons(n)  Set the maximum number of ions to simulate.
      __opentrim_driver__('setMaxNoIons', self.h_, uint64(n));
    end

    function n = threads(self)
      % threads()  Return the number of execution threads (active or configured).
      n = __opentrim_driver__('threads', self.h_);
    end

    function setNthreads(self, n)
      % setNthreads(n)  Set the number of execution threads (0 = auto).
      __opentrim_driver__('setNthreads', self.h_, uint64(n));
    end

    function n = seed(self)
      % seed()  Return the random number generator seed.
      n = __opentrim_driver__('seed', self.h_);
    end

    function ok = setSeed(self, n)
      % setSeed(n)  Set the RNG seed.
      %   Returns true on success; false if the simulation is running or
      %   has already accumulated ions.
      ok = __opentrim_driver__('setSeed', self.h_, uint32(n));
    end

  end

end
