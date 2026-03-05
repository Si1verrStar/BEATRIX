function label = scores2label(scores,labels,dim)

if nargin < 3
    dim = 2;
end

[~,idx] = max(scores,[],dim);

label = labels(idx);

end