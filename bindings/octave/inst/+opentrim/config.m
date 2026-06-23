classdef config < handle
% opentrim.config  Wrapper for mcconfig — OpenTRIM simulation configuration.
%
% Behaves like a struct with one field per JSON config section, plus
% methods for validation and serialisation.
%
% Usage:
%   cfg = opentrim.config();                   % default options
%   cfg.Run.max_no_ions = 1000;                % set a field directly
%   e   = cfg.IonBeam.energy_distribution.center;
%   cfg.from_json(fileread('sim.json'));        % load from a JSON file
%   ok  = cfg.validate();                      % check consistency
%   s   = cfg.to_json();                       % serialise to JSON string
%
% Config sections (public properties — tab-complete with cfg.<TAB>):
%   cfg.Simulation   cfg.Transport   cfg.IonBeam
%   cfg.Target       cfg.Output      cfg.Run      cfg.UserTally
%
% Note: UserTally is a JSON array.  Use cfg.from_json() to configure it
% when the array contains only one element (Octave jsondecode may drop
% the array wrapper for single-element arrays of objects).

    methods (Static, Access = private)

        function s = val2json_(v)
        % Recursively encode an Octave value to a JSON string.
        % Uses %.17g for all doubles so that large integer-valued floats
        % such as 1e30 are not corrupted by Octave's jsonencode, which
        % tries to format them as integers and overflows.
            if islogical(v)
                if isscalar(v)
                    if v; s = 'true'; else s = 'false'; end
                else
                    parts = arrayfun(@(x) opentrim.config.val2json_(x), ...
                                     v(:)', 'UniformOutput', false);
                    s = ['[' strjoin(parts, ',') ']'];
                end
            elseif isnumeric(v)
                if isempty(v)
                    s = '[]';
                elseif isscalar(v)
                    s = sprintf('%.17g', v);
                else
                    parts = arrayfun(@(x) opentrim.config.val2json_(x), ...
                                     v(:)', 'UniformOutput', false);
                    s = ['[' strjoin(parts, ',') ']'];
                end
            elseif ischar(v)
                s = ['"' strrep(strrep(v, '\', '\\'), '"', '\"') '"'];
            elseif isstruct(v)
                if isempty(v)
                    s = '[]';
                elseif numel(v) > 1
                    parts = arrayfun(@(x) opentrim.config.val2json_(x), ...
                                     v(:)', 'UniformOutput', false);
                    s = ['[' strjoin(parts, ',') ']'];
                else
                    fn = fieldnames(v);
                    kv = cellfun( ...
                        @(f) ['"' f '":' opentrim.config.val2json_(v.(f))], ...
                        fn, 'UniformOutput', false);
                    s = ['{' strjoin(kv, ',') '}'];
                end
            elseif iscell(v)
                if isempty(v)
                    s = '[]';
                else
                    parts = cellfun(@(x) opentrim.config.val2json_(x), ...
                                    v(:)', 'UniformOutput', false);
                    s = ['[' strjoin(parts, ',') ']'];
                end
            else
                error('opentrim.config: cannot JSON-encode type ''%s''', class(v));
            end
        end

    end

    properties
        Simulation = struct()
        Transport  = struct()
        IonBeam    = struct()
        Target     = struct()
        Output     = struct()
        Run        = struct()
        UserTally  = struct([])
    end

    properties (Access = private)
        h_ = uint64(0)
    end

    methods

        function self = config(h)
        % config()  — allocate a new default mcconfig
        % config(h) — wrap an existing mcconfig handle (internal use only)
            if nargin == 0
                self.h_ = __opentrim_config__('new');
            else
                self.h_ = h;
            end
            load_from_cpp_(self);
        end

        function delete(self)
            if self.h_ ~= uint64(0)
                __opentrim_config__('delete', self.h_);
                self.h_ = uint64(0);
            end
        end

        function ok = validate(self)
        % validate()  Check config consistency.  Returns true on success.
            sync_to_cpp_(self);
            ok = __opentrim_config__('validate', self.h_);
        end

        function s = to_json(self)
        % to_json()  Serialise the full configuration to a JSON string.
            sync_to_cpp_(self);
            s = __opentrim_config__('to_json', self.h_);
        end

        function from_json(self, s)
        % from_json(s)  Populate the configuration from a JSON string.
            __opentrim_config__('from_json', self.h_, s);
            load_from_cpp_(self);
        end

        function h = get_handle_(self)
        % get_handle_()  Internal: return the raw uint64 handle (syncs first).
            sync_to_cpp_(self);
            h = self.h_;
        end

    end

    methods (Access = private)

        function load_from_cpp_(self)
            d = jsondecode(__opentrim_config__('to_json', self.h_));
            self.Simulation = d.Simulation;
            self.Transport  = d.Transport;
            self.IonBeam    = d.IonBeam;
            self.Target     = d.Target;
            self.Output     = d.Output;
            self.Run        = d.Run;
            if isfield(d, 'UserTally')
                self.UserTally = d.UserTally;
            else
                self.UserTally = struct([]);
            end
        end

        function sync_to_cpp_(self)
            d = struct( ...
                'Simulation', self.Simulation, ...
                'Transport',  self.Transport,  ...
                'IonBeam',    self.IonBeam,    ...
                'Target',     self.Target,     ...
                'Output',     self.Output,     ...
                'Run',        self.Run);
            if ~isempty(self.UserTally)
                d.UserTally = self.UserTally;
            end
            __opentrim_config__('from_json', self.h_, opentrim.config.val2json_(d));
        end

    end

end
