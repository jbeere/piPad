(function($) {

	function mouseUp() {
		$(this).data('sense').pos[0].a=0;
		mouseXY.apply(this);
	}

	function touchUp(e) {
		if (!e) var e = event;
		var pos = $(this).data('sense').pos;
		var i = 0;
		for (; i < e.targetTouches.length; i++) {
			pos[i].a = 1;
		}
		for (; i < pos.length; i++) {
			pos[i].a = 0;
		}
		render.apply(this);
	}

	function mouseDown() {
		$(this).data('sense').pos[0].a=1;
		mouseXY.apply(this);
	}

	function touchDown() {
		touchXY.apply(this);
	}
	
	function mouseXY(e) {
		if (!e) var e = event;
		var pos = $(this).data('sense').pos[0];
		pos.x = e.pageX - this.offsetLeft;
		pos.y = e.pageY - this.offsetTop;
		render.apply(this);
	}

	function touchXY(e) {
		if (!e) var e = event;
		e.preventDefault();
		var pos = $(this).data('sense').pos;
		var i = 0;
		for (; i < e.targetTouches.length; i++) {
			pos[i].a = 1;
			pos[i].x = e.targetTouches[i].pageX - this.offsetLeft;
			pos[i].y = e.targetTouches[i].pageY - this.offsetTop;
		}
		for (; i < pos.length; i++) {
			pos[i].a = 0;
		}
		render.apply(this);
	}

	function render() {
		var data = $(this).data('sense');
		var dim = data.dim;
		var ctx = this.getContext('2d');
		var dia = 40;
		ctx.clearRect(0,0,dim.width,dim.height);
		for (var i = 0; i < data.pos.length; i++) {
			var pos = data.pos[i];
			if (!pos.a) return;
			$(this).trigger('move', { id: i, pos: pos });
			ctx.beginPath();
			ctx.arc(pos.x,pos.y,dia,0,Math.PI*2,true);
			ctx.closePath();
			ctx.strokeStyle='#ffffff';
			ctx.lineWidth = 5;
			ctx.stroke();
		}
	} 

	var methods = {
		init: function(options) {
			return this.each(function(){
				var $this = $(this);
				var cWidth = this.offsetWidth;
				var cHeight = this.offsetHeight;
				var canvas = $('<canvas width="'+cWidth+'" height="'+cHeight+'"></canvas>');
				$this.append(canvas);
				canvas.data('sense', {
					pos: [{a:0,x:0,y:0},{a:0,x:0,y:0},{a:0,x:0,y:0},{a:0,x:0,y:0}], 
					dim: {width:cWidth,height:cHeight}
				});
				canvas[0].addEventListener('mousedown', mouseDown, false);
				canvas[0].addEventListener('mousemove', mouseXY, false);
				canvas[0].addEventListener('touchstart', touchDown, false);
				canvas[0].addEventListener('touchmove', touchXY, true);
				canvas[0].addEventListener('touchend', touchUp, false);
				document.body.addEventListener('mouseup', function() { mouseUp.apply(canvas[0]); }, false );
				document.body.addEventListener('touchcancel', function() { touchUp.apply(canvas[0]); }, false );
			});
		}
	};

	$.fn.multisense = function(method) {
		if (methods[method]) {
			return methods[method].apply(this, Array.prototype.slice.call(arguments, 1));
		}
		else if (typeof method === 'object' || !method) {
			return methods.init.apply(this, arguments);
		}
		else {
			$.error('Method ' + method + ' does not exist in sense');
		}
	}


})(jQuery);
