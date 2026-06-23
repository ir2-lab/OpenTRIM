classdef info < handle
  % opentrim.info  Wrapper for mcinfo — read-only view of simulation results.
  %
  % Obtain an info object from a completed driver:
  %   res = d.info();
  %
  % Navigate and retrieve data by JSON path:
  %   x          = res.get('/target/grid/x');          % 1-D double array
  %   V          = res.get('/tally/damage_events/vacancies');
  %   [V, dV]    = res.get('/tally/damage_events/vacancies'); % with SEM error
  %   sub        = res.get('/tally');                  % returns opentrim.info
  %   label      = res.get('/Simulation/title');       % string
  %
  % Data types returned by get():
  %   group         → opentrim.info (sub-tree)
  %   real64/real32 → double/single NDArray (row vector for 1-D data)
  %   uint64        → uint64 NDArray
  %   string/json   → char (scalar) or cell array of char
  %   tally_score   → [values, errors] when called with two outputs
  %
  % LIFETIME: this object holds an internal pointer to the originating
  % opentrim.driver.  The driver must outlive all info objects derived from it.

  properties (Access = private)
    h_ = uint64(0)
  end

  methods

    function self = info(drv_h, is_subtree)
      % info(drv_h)           — construct from a driver handle (uint64)
      % info(drv_h, true)     — wrap an existing mcinfo subtree handle
      %                         (used internally by get() for group nodes)
      if nargin == 0
        return; % empty object; h_ stays uint64(0)
      elseif nargin >= 2 && is_subtree
        self.h_ = drv_h; % drv_h is already an mcinfo handle
      else
        self.h_ = __opentrim_info__('new', drv_h);
      end

    end

    function delete(self)

      if self.h_ ~= uint64(0)
        __opentrim_info__('delete', self.h_);
        self.h_ = uint64(0);
      end

    end

    function varargout = get(self, path)
      % get(path)       — retrieve data at path; see class doc for types.
      % [V,dV] = get(path) — for tally_score nodes, also return SEM errors.
      nout = max(nargout, 1);

      % Check node type: group or data
      if __opentrim_info__('is_group', self.h_, path)
        h = __opentrim_info__('get', self.h_, path, 1);
        varargout{1} = opentrim.info(h, true);
      else
        varargout = cell(1, nout);
        [varargout{:}] = __opentrim_info__('get', self.h_, path, nout);
      end

    end

    function s = description(self, path)
      % description()        — description of the root node
      % description(path)    — description of the node at path
      if nargin < 2,
        s = __opentrim_info__('description', self.h_);
      else
        s = __opentrim_info__('description', self.h_, path);
      end

    end

    function s = keys(self)
      % keys()        — the names of the child nodes
      s = __opentrim_info__('keys', self.h_);
    end

    function s = dump(self, path)
      % disp()        — Display node info
      if nargin < 2,
        k = keys(self);

        for i = 1:length(k)
          disp(dump(self, k{i}));
        end

        s = [];
      else
        s = __opentrim_info__('dump', self.h_, path);
      end

    end

    function s = is_group(self, path)
      % is_group(path) — True if node at path is a group node
      if nargin < 2,
        error("is_group: specify node path");
      else
        s = __opentrim_info__('is_group', self.h_, path);
      end

    end

  end

end
